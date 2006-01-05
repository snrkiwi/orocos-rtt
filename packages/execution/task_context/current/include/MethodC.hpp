#ifndef ORO_EXECUTION_METHODC_HPP
#define ORO_EXECUTION_METHODC_HPP

#include <string>
#include "DataSource.hpp"
#include "TaskAttribute.hpp"

namespace ORO_Execution
{
    class GlobalMemberFactory;

    /**
     * A user friendly method to a TaskContext.
     */
    class MethodC
    {
        /**
         * The 'd' pointer pattern.
         */
        class D;
        D* d;
        DataSourceBase::shared_ptr m;
    public:
        /**
         * The default constructor.
         * Make a copy from another MethodC object
         * in order to make it usable.
         */
        MethodC();

        /**
         * The constructor.
         * @see GlobalMethodFactory
         */
        MethodC( const GlobalMemberFactory* gcf, const std::string& obj, const std::string& name);

        /**
         * A MethodC is copyable by value.
         */
        MethodC(const MethodC& other);

        ~MethodC();

        /**
         * Add a datasource argument to the Method.
         * @param a A DataSource which contents are consulted each time
         * when execute() is called.
         */
        MethodC& arg( DataSourceBase::shared_ptr a );

        /**
         * Add a constant argument to the Method.
         * @param a A value of which a copy is made and this value is used each time
         * in execute().
         */
        template< class ArgT >
        MethodC& argC( const ArgT a )
        {
            return this->arg(DataSourceBase::shared_ptr( new ConstantDataSource<ArgT>( a ) ) );
        }

        /**
         * Add an argument by reference to the Method.
         * @param a A value of which the reference is used and re-read each time
         * the method is executed. Thus if the contents of the source of \a a changes,
         * execute() will use the new contents.
         */
        template< class ArgT >
        MethodC& arg( ArgT& a )
        {
            return this->arg(DataSourceBase::shared_ptr( new ReferenceDataSource<ArgT>( a ) ) );
        }

        /**
         * Store the result of the method in a task's attribute.
         * @param r A task attribute in which the result is stored.
         */
        MethodC& ret( TaskAttributeBase* r );

        /**
         * Store the result of the method in variable.
         * @param r A reference to the variable in which the result is stored.
         */
        template< class RetT >
        MethodC& ret( RetT& r )
        {
            // this is semantically valid wrt TaskAttribute::copy().
            TaskAttributeBase* ta = new TaskAttribute<RetT>( new ReferenceDataSource<RetT>(r));
            this->ret( ta );
            delete ta;
            return *this;
        }

        /**
         * Execute the contained method.
         */
        bool execute();

        /**
         * Reset the method.
         * Required before invoking execute() a second time.
         */
        void reset();
    };
}

#endif