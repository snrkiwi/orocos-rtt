/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:21 CET 2004  ConditionCompare.hpp

                        ConditionCompare.hpp -  description
                           -------------------
    begin                : Mon January 19 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public                   *
 *   License as published by the Free Software Foundation;                 *
 *   version 2 of the License.                                             *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction.  Specifically, if other files   *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the executable file might be covered by the GNU General   *
 *   Public License.                                                       *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef CONDITIONCOMPARE_HPP
#define CONDITIONCOMPARE_HPP

#include "ConditionInterface.hpp"
#include <typeinfo>

#include "DataSource.hpp"

namespace RTT
{

  /**
   * A general compare condition.  This compares two variables of type
   * T, which it gets from two DataSource<T>, using the
   * compare_op given.  You should use std::less, std::less_equal,
   * std::greater, std::greater_equal, std::equal_to and
   * std::not_equal_to or other binary predicate functors as
   * compare_op..
   * @deprecated Remove this low-level class since it's nowhere used.
   */
  template<typename T, typename compare_op>
  class ConditionCompare
    : public ConditionInterface
  {
    typename DataSource<T>::shared_ptr mdata1;
    typename DataSource<T>::shared_ptr mdata2;
  public:
    ConditionCompare( DataSource<T>* data1, DataSource<T>* data2 )
      : mdata1( data1 ), mdata2( data2 )
      {
      }

    bool evaluate();
    ConditionInterface* copy() const
      {
        return new ConditionCompare<T, compare_op>(
          mdata1.get(), mdata2.get() );
      };
    std::string toString()
      {
        return std::string( "compare<" ) + typeid( T ).name() +
          ", " + typeid( compare_op ).name() + ">";
      };
  };

  template<typename T, typename compare_op>
  bool ConditionCompare<T, compare_op>::evaluate()
  {
    compare_op op;
    return op( mdata1->get(), mdata2->get() );
  };
}

#endif
