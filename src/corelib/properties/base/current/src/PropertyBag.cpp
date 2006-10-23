/***************************************************************************
  tag: Peter Soetens  Mon May 10 19:10:30 CEST 2004  PropertyBag.cxx 

                        PropertyBag.cxx -  description
                           -------------------
    begin                : Mon May 10 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be
 
 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

#ifdef ORO_PRAGMA_INTERFACE
#pragma implementation
#endif
#include "rtt/PropertyBag.hpp"
#include "rtt/Property.hpp"
#include "rtt/Logger.hpp"

namespace RTT
{

    PropertyBag::PropertyBag( )
        : mproperties(), type("type_less")
    {}

    PropertyBag::PropertyBag( const std::string& _type)
        : mproperties(), type(_type)
    {}

    PropertyBag::PropertyBag( const PropertyBag& orig)
        : mproperties( orig.getProperties() ), type( orig.getType() )
    {
    }

    void PropertyBag::add(PropertyBase *p)
    {
        this->addProperty(p);
    }

    void PropertyBag::remove(PropertyBase *p)
    {
        this->removeProperty(p);
    }

    bool PropertyBag::addProperty(PropertyBase *p)
    {
        if ( ! p->ready() )
            return false;
        mproperties.push_back(p);
        return true;
    }

    bool PropertyBag::removeProperty(PropertyBase *p)
    {
        iterator i = mproperties.begin();
        i = mproperties.end();
        i = std::find(mproperties.begin(), mproperties.end(), p);
        if ( i != mproperties.end() ) {
            mproperties.erase(i);
            return true;
        }
        return false;
    }

    void PropertyBag::clear()
    {
        mproperties.clear();
    }

    void PropertyBag::list(std::vector<std::string> &names) const
    {
        for (
             const_iterator i = mproperties.begin();
             i != mproperties.end();
             i++ )
            {
                names.push_back( (*i)->getName() );
            }
    }

    std::vector<std::string> PropertyBag::list() const
    {
        std::vector<std::string> names;
        for (
             const_iterator i = mproperties.begin();
             i != mproperties.end();
             i++ )
            {
                names.push_back( (*i)->getName() );
            }
        return names;
    }

    void PropertyBag::identify( PropertyIntrospection* pi ) const
    {
        for ( const_iterator i = mproperties.begin();
              i != mproperties.end();
              i++ )
            {
                (*i)->identify(pi);
            }
    }

    void PropertyBag::identify( PropertyBagVisitor* pi ) const
    {
        for ( const_iterator i = mproperties.begin();
              i != mproperties.end();
              i++ )
            {
                (*i)->identify(pi);
            }
    }

    PropertyBase* PropertyBag::find(const std::string& name) const
    {
        const_iterator i( std::find_if(mproperties.begin(), mproperties.end(), std::bind2nd(PropertyBag::FindProp(), name ) ) );
        if ( i != mproperties.end() )
            return ( *i );
        return 0;
    }

    PropertyBag& PropertyBag::operator=(const PropertyBag& orig)
    {
        mproperties.clear();

        const_iterator i = orig.getProperties().begin();
        while (i != orig.getProperties().end() )
            {
                add( (*i) );
                ++i;
            }
        this->setType( orig.getType() );
        return *this;
    }

    PropertyBag& PropertyBag::operator<<=(const PropertyBag& source)
    {
        //iterate over orig, update or clone PropertyBases
        const_iterator it(source.getProperties().begin());
        while ( it != source.getProperties().end() )
            {
                PropertyBase* mine = find( (*it)->getName() );
                if (mine != 0)
                    remove(mine);
                add( (*it) );
                ++it;
            }
        this->setType( source.getType() );
        return *this;
    }

    PropertyBase* findProperty(const PropertyBag& bag, const std::string& nameSequence, const std::string& separator)
    {
        PropertyBase* result;
        Property<PropertyBag>*  result_bag;
        std::string token;
        std::string::size_type start = 0;
        if ( separator.length() != 0 && nameSequence.find(separator) == 0 ) // detect 'root' attribute
            start = separator.length();
        std::string::size_type len = nameSequence.find(separator, start);
        if (len != std::string::npos) {
            token = nameSequence.substr(start,len-start);
            start = len + separator.length();      // reset start to next token.
            if ( start >= nameSequence.length() )
                start = std::string::npos;
        }
        else {
            token = nameSequence.substr(start);
            start = std::string::npos; // do not look further.
        }
        result = bag.find(token);
        if (result != 0 ) // get the base with this name
        {
            result_bag = dynamic_cast<Property<PropertyBag>*>(result);
            if ( result_bag != 0 && start != std::string::npos ) {
                return findProperty( result_bag->rvalue(), nameSequence.substr( start ), separator );// a bag so search recursively
            }
            else
                return result; // not a bag, so it is a result.
        }
        return 0; // failure
    }

    bool refreshProperties(const PropertyBag& target, const PropertyBag& source, bool allprops)
    {
        Logger::In in("refreshProperties");
        //iterate over source, update PropertyBases
        PropertyBag::const_iterator it( target.getProperties().begin() );
        bool failure = false;
        while ( it != target.getProperties().end() )
        {
            PropertyBase* srcprop = source.find( (*it)->getName() );
            PropertyBase* tgtprop = *it;
            if (srcprop != 0)
            {
                //std::cout <<"*******************refresh"<<std::endl;
                if ( tgtprop->refresh( srcprop ) == false ) {
                    // try composition:
                    if ( tgtprop->getTypeInfo()->composeType( srcprop->getDataSource(), tgtprop->getDataSource() ) == false ) {
                        log(Error) << "Could not update, nor compose Property "
                                   << tgtprop->getType() << " "<< srcprop->getName()
                                   << ": type mismatch, can not refresh with type "
                                   << srcprop->getType() << endlog();
                        failure = true;
                    } else {
                        log(Debug) << "Composed Property "
                                      << tgtprop->getType() << " "<< srcprop->getName()
                                      << " from type "  << srcprop->getType() << endlog();
                    }
                }
                // ok.
            } else if (allprops) {
                log(Error) << "Could not find Property "
                           << tgtprop->getType() << " "<< tgtprop->getName()
                           << " in source."<< endlog();
                failure = true;
            }
            ++it;
        }
        return !failure;
    }

    bool refreshProperty(const PropertyBag& target, const PropertyBase& source)
    {
        PropertyBase* target_prop;
        // dynamic_cast ?
        if ( 0 != (target_prop = target.find( source.getName() ) ) )
            {
                return target_prop->refresh( &source );
            }
        return false;
    }

    bool copyProperties(PropertyBag& target, const PropertyBag& source)
    {
        // Make a full deep copy.
        //iterate over source, clone all PropertyBases
        PropertyBag::const_iterator it( source.getProperties().begin() );
        while ( it != source.getProperties().end() )
        {
            // step 1 : clone a new instance (non deep copy)
            PropertyBase* temp = (*it)->create();
            // step 2 : deep copy clone with original.
            temp->copy( *it );
            // step 3 : add result to target bag.
            target.add( temp );
            ++it;
        }
        return true;
    }

    bool updateProperties(PropertyBag& target, const PropertyBag& source)
    {
        // check type consistency...
        // this was probably intended to convert old xml formats to new ones.
        // but deleting in target seems just very wrong...
#if 0
        if ( target.getType() != source.getType() ) {
            // if different types, discard the old contents and
            // put in the new contents...
            // update type info...
            // delete old type contents...
            flattenPropertyBag(target);
            deleteProperties(target);
            // now continue 'updating'
        }
#endif

        target.setType( source.getType() );

        // Make an updated if present, create if not present
        //iterate over source, update or clone PropertyBases
        PropertyBag::const_iterator it( source.getProperties().begin() );
        while ( it != source.getProperties().end() )
        {
            PropertyBase* mine = target.find( (*it)->getName() );
            if (mine != 0) {
#ifndef NDEBUG
                Logger::log() << Logger::Debug;
                Logger::log() << "updateProperties: updating Property "
                              << (*it)->getType() << " "<< (*it)->getName()
                              << "." << Logger::endl;
#endif
                  // no need to make new one, just update existing one
                if ( mine->update( (*it) ) == false ) {
                    // try composition:
                    if ( mine->getTypeInfo()->composeType( (*it)->getDataSource(), mine->getDataSource() ) == false ) {
                        Logger::log() << Logger::Error;
                        Logger::log() << "updateProperties: Could not update, nor compose Property "
                                      << mine->getType() << " "<< (*it)->getName()
                                      << ": type mismatch, can not update with type "
                                      << (*it)->getType() << Logger::endl;
                        return false;
                    }
                }
                // ok.
            }
            else
            {
#ifndef NDEBUG
                Logger::log() << Logger::Debug;
                Logger::log() << "updateProperties: created Property "
                              << (*it)->getType() << " "<< (*it)->getName()
                              << "." << Logger::endl;
#endif
                // step 1 : clone a new instance (non deep copy)
                PropertyBase* temp = (*it)->create();
                // step 2 : deep copy clone with original, will never fail.
                temp->update( (*it) );
                // step 3 : add result to target bag.
                target.add( temp );
            }
            ++it;
        }
        return true;
    }

    bool updateProperty(PropertyBag& target, const PropertyBag& source, const std::string& name, const std::string& separator)
    {
        Logger::In in("updateProperty");
        // this code has been copied&modified from findProperty().
        PropertyBase* source_walker;
        PropertyBase* target_walker;
        std::string token;
        std::string::size_type start = 0;
        if ( separator.length() != 0 && name.find(separator) == 0 ) // detect 'root' attribute
            start = separator.length();
        std::string::size_type len = name.find(separator, start);
        if (len != std::string::npos) {
            token = name.substr(start,len-start);
            start = len + separator.length();      // reset start to next token.
            if ( start >= name.length() )
                start = std::string::npos;
        }
        else {
            token = name.substr(start);
            start = std::string::npos; // do not look further.
        }
        source_walker = source.find(token);
        target_walker = target.find(token);
        if (source_walker != 0 )
        {
            if ( target_walker == 0 ) {
                // if not present in target, create it !
                target_walker = source_walker->create();
                target.addProperty( target_walker );
            }
            Property<PropertyBag>*  source_walker_bag;
            Property<PropertyBag>*  target_walker_bag;
            source_walker_bag = dynamic_cast<Property<PropertyBag>*>(source_walker);
            target_walker_bag = dynamic_cast<Property<PropertyBag>*>(target_walker);
            if ( source_walker_bag != 0 && start != std::string::npos ) {
                if ( target_walker_bag == 0 ) {
                    log(Error) << "Property '"<<target_walker->getName()<<"' is not a PropertyBag !"<<endlog();
                    return false;
                }
                return updateProperty( target_walker_bag->value(), source_walker_bag->rvalue(), name.substr( start ), separator );// a bag so search recursively
            }
            else {
                // found it, update !
                if (target_walker->update(source_walker) == false ) {
                    // try composition:
                    if ( target_walker->getTypeInfo()->composeType( source_walker->getDataSource(), target_walker->getDataSource() ) == false ) {
                        log(Error) << "Could not update nor compose Property "
                                   << target_walker->getType() << " "<< target_walker->getName()
                                   << ": type mismatch, can not update with type "
                                   << source_walker->getType() << Logger::endl;
                    }
                }
                log(Debug) << "Found Property '"<<target_walker->getName() <<"': update done." << endlog();
                return true;
            }
        } else {
            // error wrong path, not present in source !
            log(Error) << "Property '"<< token <<"' is not present in the source PropertyBag !"<<endlog();
            return false;
        }
        // not reached.
        return false; // failure
    }

    void deleteProperties(PropertyBag& target)
    {
        //recursive delete.
        PropertyBag::const_iterator it( target.getProperties().begin() );
        while ( it != target.getProperties().end() )
        {
            delete (*it);
            ++it;
        }
        target.clear();
    }

    void deletePropertyBag(PropertyBag& target)
    {
        //iterate over target, delete PropertyBases
        PropertyBag::const_iterator it( target.getProperties().begin() );
        while ( it != target.getProperties().end() )
        {
            Property<PropertyBag>* result = dynamic_cast< Property<PropertyBag>* >( *it );
            if ( result != 0 )
                deletePropertyBag( result->value() );
            delete (*it);
            ++it;
        }
        target.clear();
    }

    void flattenPropertyBag(PropertyBag& target, const std::string& separator)
    {
        //iterate over target, recursively flatten bags.
        Property<PropertyBag>* result;
        PropertyBag::const_iterator it( target.getProperties().begin() );
        while ( it != target.getProperties().end() )
        {
            result = dynamic_cast< Property<PropertyBag>* >( *it );
            if ( result != 0 )
            {
                flattenPropertyBag( result->value(), separator );// a bag so flatten recursively
                // copy all elements from result to target.
                PropertyBag::const_iterator flat_it( result->value().getProperties().begin() ) ;
                if ( flat_it != result->value().getProperties().end() )
                {
                    while (flat_it != result->value().getProperties().end() )
                    {
                        (*flat_it)->setName( result->getName() + separator + (*flat_it)->getName() );
                        target.add( *flat_it );
                        result->value().remove( *flat_it );
                        flat_it =  result->value().getProperties().begin();
                    }
                    it = target.getProperties().begin(); // reset iterator
                    continue;                            // do not increase it
                } 
                // the bag is empty now, but it must stay in target.
            }
            ++it;
        }
    }

}