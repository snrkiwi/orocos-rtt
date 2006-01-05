#ifndef ORO_LIST_LOCK_FREE_HPP
#define ORO_LIST_LOCK_FREE_HPP
 
#include <vector>
#include "os/oro_atomic.h"
#include "os/CAS.hpp"
#include <boost/intrusive_ptr.hpp>

#ifdef ORO_PRAGMA_INTERFACE
#pragma interface
#endif

namespace ORO_CoreLib
{
    namespace detail {
        struct IntrusiveStorage 
        {
            atomic_t ref;
            IntrusiveStorage() {
                atomic_set(&ref,0);
            }
            virtual ~IntrusiveStorage() {
            }
        };
    }
}


void intrusive_ptr_add_ref(ORO_CoreLib::detail::IntrusiveStorage* p );
void intrusive_ptr_release(ORO_CoreLib::detail::IntrusiveStorage* p );

namespace ORO_CoreLib
{
    /**
     * A \a simple lock-free list implementation to \a append or \a erase
     * data of type \a T.
     * No memory allocation is done during read or write, but the maximum number
     * of threads which can access this object is defined by
     * MAX_THREADS.
     * @param T The value type to be stored in the list.
     * Example : ListLockFree<A> is a list which holds values of type A.
     */
    template< class T>
    class ListLockFree
    {
    public:
        /** 
         * @brief The maximum number of threads.
         *
         * The number of threads which may concurrently access this buffer.
         */
        const unsigned int MAX_THREADS;

        typedef T value_t;
    private:
        typedef std::vector<value_t> BufferType;
        typedef typename BufferType::iterator Iterator;
        typedef typename BufferType::const_iterator CIterator;
        struct Item {
            Item()  {
                //ATOMIC_INIT(count);
                atomic_set(&count,-1);
            }
            mutable atomic_t count;  // refcount
            BufferType data;
        };

        struct StorageImpl : public detail::IntrusiveStorage 
        {
            Item* items;
            StorageImpl(size_t alloc) : items( new Item[alloc] ) {
            }
            ~StorageImpl() {
                delete[] items;
            }
            Item& operator[](int i) {
                return items[i];
            }
        };

        /**
         * The intrusive_ptr is far more thread-safe than the
         * shared_ptr for 'reads' during 'assignments'.
         */
        typedef boost::intrusive_ptr<StorageImpl> Storage;

        Storage newStorage(size_t alloc, size_t items, bool init = true)
        {
            Storage st( new StorageImpl(alloc) );
            for (unsigned int i=0; i < alloc; ++i) {
                (*st)[i].data.reserve( items ); // pre-allocate
            }
            // bootstrap the first list :
            if (init) {
                active = &(*st)[0];
                atomic_inc( &active->count );
            }

            return st;
        }

        Storage bufs;
        Item* volatile active;

        // each thread has one 'working' buffer, and one 'active' buffer
        // lock. Thus we require to allocate twice as much buffers as threads,
        // for all the locks to succeed in a worst case scenario.
        inline size_t BufNum() const {
            return MAX_THREADS * 2;
        }

        size_t required;
    public:
        /**
         * Create a lock-free list wich can store \a lsize elements.
         * @param lsize the capacity of the list.
         * @param threads the number of threads which may concurrently
         * read or write this buffer. Defaults to ORONUM_OS_MAX_THREADS, but you
         * may lower this number in case not all threads will read this buffer.
         * A lower number will consume less memory.
'        */
        ListLockFree(unsigned int lsize, unsigned int threads = ORONUM_OS_MAX_THREADS )
            : MAX_THREADS( threads ), required(0)
        {
            const unsigned int BUF_NUM = BufNum();
            bufs = newStorage( BUF_NUM, lsize );
        }

        ~ListLockFree() {
        }

        size_t capacity() const
        {
            size_t res;
            Storage st;
            Item* orig = lockAndGetActive(st);
            res = orig->data.capacity();
            atomic_dec( &orig->count ); // lockAndGetActive
            return res;
        }

        size_t size() const
        {
            size_t res;
            Storage st;
            Item* orig = lockAndGetActive(st);
            res = orig->data.size();
            atomic_dec( &orig->count ); // lockAndGetActive
            return res;
        }

        bool empty() const
        {
            bool res;
            Storage st;
            Item* orig = lockAndGetActive(st);
            res = orig->data.empty();
            atomic_dec( &orig->count ); // lockAndGetActive
            return res;
        }

        /**
         * Grow the capacity to contain at least n additional items.
         * This method tries to avoid too much re-allocations, by
         * growing a bit more than required every N invocations and
         * growing nothing in between.
         * @param items The number of items to at least additionally reserve.
         */
        void grow(size_t items = 1) {
            required += items;
            if (required > this->capacity()) {
                this->reserve( required*2 );
            }
        }
        /**
         * Shrink the capacity with at most n items.
         * This method does not actually free memory, it just prevents
         * a (number of) subsequent grow() invocations to allocate more
         * memory.
         * @param items The number of items to at most remove from the capacity.
         */
        void shrink(size_t items = 1) {
            required -= items;
        }

        /**
         * Reserve a capacity for this list.
         * If you wish to invoke this method concurrently, guard it
         * with a mutex. The other methods may be invoked concurrently
         * with this method.
         * @param lsize the \a minimal number of items this list will be
         * able to hold. Will not drop below the current capacity()
         * and this method will do nothing if \a lsize < this->capacity().
         */
        void reserve(size_t lsize)
        {
            if (lsize <= this->capacity() )
                return;

            const unsigned int BUF_NUM = BufNum();
            Storage res( newStorage(BUF_NUM, lsize, false) );

            // init the future 'active' buffer.
            Item* nextbuf = &(*res)[0];
            atomic_inc( &nextbuf->count );

            // temporary for current active buffer.
            Item* orig = 0;

            // prevent current bufs from deletion.
            // will free upon return.
            Storage save = bufs;
            // active points at old, bufs points at new:
            // first the refcount is added to res, then
            // bufs' pointer is switched to res' pointer,
            // and stored in a temporary. Then the temp
            // is destructed and decrements bufs' old reference.
            bufs = res;
            // from now on, any findEmptyBuf will use the new bufs,
            // unless the algorithm was entered before the switch.
            // then, it will write the result to the old buf.
            // if it detects we updated active, it will find an
            // empty buf in the new buf. If it gets there before
            // our CAS, our CAS will fail and we try to recopy
            // everything. This retry may be unnessessary 
            // if the data already is in the new buf, but for this
            // cornercase, we must be sure.

            // copy active into new:
            do {
                if (orig)
                    atomic_dec(&orig->count);
                orig = lockAndGetActive(); // active is guaranteed to point in valid buffer ( save or bufs )
                nextbuf->data.clear();
                Iterator it( orig->data.begin() );
                while ( it != orig->data.end() ) {
                    nextbuf->data.push_back( *it );
                    ++it;
                }
                // see explanation above: active could have changed,
                // and still point in old buffer. we could check this
                // with pointer arithmetics, but this is not a performant
                // method.
            } while ( ORO_OS::CAS(&active, orig, nextbuf ) == false);
            // now,
            // active is guaranteed to point into bufs.
            assert( pointsTo( active, bufs ) );

            atomic_dec( &orig->count ); // lockAndGetActive
            atomic_dec( &orig->count ); // ref count
        }

        void clear()
        {
            Storage bufptr;
            Item* orig(0);
            Item* nextbuf(0);
            int items = 0;
            do {
                if (orig) {
                    atomic_dec(&orig->count);
                    atomic_dec(&nextbuf->count);
                }
                orig = lockAndGetActive(bufptr);
                items = orig->data.size();
                nextbuf = findEmptyBuf(bufptr); // find unused Item in bufs
                nextbuf->data.clear();
            } while ( ORO_OS::CAS(&active, orig, nextbuf ) == false );
            atomic_dec( &orig->count ); // lockAndGetActive
            atomic_dec( &orig->count ); // ref count
        }

        /**
         * Append a single value to the list.
         * @param d the value to write
         * @return false if the list is full.
         */
        bool append( value_t item ) 
        {
            Item* orig=0;
            Storage bufptr;
            Item* usingbuf(0);
            do {
                if (orig) {
                    atomic_dec(&orig->count);
                    atomic_dec(&usingbuf->count);
                }
                orig = lockAndGetActive( bufptr );
                if ( orig->data.size() == orig->data.capacity() ) { // check for full
                    atomic_dec( &orig->count );
                    return false;
                }
                usingbuf = findEmptyBuf( bufptr ); // find unused Item in bufs
                usingbuf->data = orig->data;
                usingbuf->data.push_back( item );
            } while ( ORO_OS::CAS(&active, orig, usingbuf ) ==false);
            atomic_dec( &orig->count ); // lockAndGetActive()
            atomic_dec( &orig->count ); // set list free
            return true;
        }

        /**
         * Returns the first element of the list.
         */
        value_t front() const
        {
            Storage bufptr;
            Item* orig = lockAndGetActive(bufptr);
            value_t ret(orig->data.front());
            atomic_dec( &orig->count ); //lockAndGetActive
            return ret;
        }

        /**
         * Returns the last element of the list.
         */
        value_t back() const
        {
            Storage bufptr;
            Item* orig = lockAndGetActive(bufptr);
            value_t ret(orig->data.back());
            atomic_dec( &orig->count ); //lockAndGetActive
            return ret;
        }

        /**
         * Append a sequence of values to the list. 
         * @param items the values to append.
         * @return the number of values written (may be less than d.size())
         */
        size_t append(const std::vector<T>& items)
        {
            Item* usingbuf(0);
            Item* orig=0;
            int towrite  = items.size();
            Storage bufptr;
            do {
                if (orig) {
                    atomic_dec(&orig->count);
                    atomic_dec(&usingbuf->count);
                }
                    
                orig = lockAndGetActive( bufptr );
                int maxwrite = orig->data.capacity() - orig->data.size();
                if ( maxwrite == 0 ) {
                    atomic_dec( &orig->count ); // lockAndGetActive()
                    return 0;
                }
                if ( towrite > maxwrite )
                    towrite = maxwrite;
                usingbuf = findEmptyBuf( bufptr ); // find unused Item in bufs
                usingbuf->data = orig->data;
                usingbuf->data.insert( usingbuf->data.end(), items.begin(), items.begin() + towrite );
            } while ( ORO_OS::CAS(&active, orig, usingbuf ) ==false );
            atomic_dec( &orig->count ); // lockAndGetActive()
            atomic_dec( &orig->count ); // set list free
            return towrite;
        }


        /**
         * Erase a value from the list.
         * @param item is to be erased from the list.
         * @return true if found and erased.
         */
        bool erase( value_t item )
        {
            Item* orig=0;
            Item* nextbuf(0);
            Storage bufptr;
            do {
                if (orig) {
                    atomic_dec(&orig->count);
                    atomic_dec(&nextbuf->count);
                }
                orig = lockAndGetActive( bufptr ); // find active in bufptr
                // we do this in the loop because bufs can change.
                nextbuf = findEmptyBuf( bufptr ); // find unused Item in same buf.
                nextbuf->data.clear();
                Iterator it( orig->data.begin() );
                while (it != orig->data.end() && !( *it == item ) ) {
                    nextbuf->data.push_back( *it );
                    ++it;
                }
                if ( it == orig->data.end() ) {
                    atomic_dec( &orig->count );
                    atomic_dec( &nextbuf->count );
                    return false; // item not found.
                }
                ++it; // skip item.
                while ( it != orig->data.end() ) {
                    nextbuf->data.push_back( *it );
                    ++it;
                }
            } while ( ORO_OS::CAS(&active, orig, nextbuf ) ==false );
            atomic_dec( &orig->count ); // lockAndGetActive
            atomic_dec( &orig->count ); // ref count
            return true;
        }

        /**
         * Apply a function to the elements of the whole list.
         * @param func The function to apply.
         */
        template<class Function>
        void apply(Function func )
        {
            Storage st;
            Item* orig = lockAndGetActive(st);
            Iterator it( orig->data.begin() );
            while ( it != orig->data.end() ) {
                func( *it );
                ++it;
            }
            atomic_dec( &orig->count ); //lockAndGetActive
        }

    private:
        /**
         * Item returned is guaranteed to point into bufptr.
         */
        Item* findEmptyBuf(Storage& bufptr) {
            // These two functions are copy/pasted from BufferLockFree.
            // If MAX_THREADS is large enough, this will always succeed :
            Item* start = &(*bufptr)[0];
            while( true ) {
                if ( atomic_inc_and_test( &start->count ) )
                    break;
                atomic_dec( &start->count );
                ++start;
                if (start == &(*bufptr)[0] + BufNum() )
                    start = &(*bufptr)[0]; // in case of races, rewind
            }
            assert( pointsTo(start, bufptr) );
            return start; // unique pointer across all threads
        }

        /**
         * Item returned is guaranteed to point into bufptr.
         */
        Item* lockAndGetActive(Storage& bufptr) const {
            // This is a kind-of smart-pointer implementation
            // We could move it into Item itself and overload operator=
            Item* orig=0;
            do {
                if (orig)
                    atomic_dec( &orig->count );
                bufptr = bufs;
                orig = active;
                // also check that orig points into bufptr.
                if ( pointsTo(orig, bufptr) )
                    atomic_inc( &orig->count );
                else {
                    orig = 0;
                }
                // this synchronisation point is 'aggressive' (a _sufficient_ condition)
                // if active is still equal to orig, the increase of orig->count is
                // surely valid, since no contention (change of active) occured.
            } while ( active != orig );
            assert( pointsTo(orig, bufptr) );
            return orig;
        }

        /**
         * Only to be used by reserve() !
         * Caller must guarantee that active points to valid memory.
         */
        Item* lockAndGetActive() const {
            // only operates on active's refcount.
            Item* orig=0;
            do {
                if (orig)
                    atomic_dec( &orig->count );
                orig = active;
                atomic_inc( &orig->count );
                // this synchronisation point is 'aggressive' (a _sufficient_ condition)
                // if active is still equal to orig, the increase of orig->count is
                // surely valid, since no contention (change of active) occured.
            } while ( active != orig );
            return orig;
        }

        inline bool pointsTo( Item* p, const Storage& bf ) const {
            return p >= &(*bf)[0] && p <= &(*bf)[ BufNum() - 1 ];
        }

    };

    extern template class ListLockFree<double>;

}

#endif