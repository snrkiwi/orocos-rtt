/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:26 CET 2004  DataSourceCondition.hpp

                        DataSourceCondition.hpp -  description
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

#ifndef DATASOURCECONDITION_HPP
#define DATASOURCECONDITION_HPP

#include "DataSource.hpp"

namespace RTT
{
  class ConditionInterface;

  /**
   * A class that wraps a Condition in a DataSource<bool>
   * interface.
   */
  class RTT_API DataSourceCondition
    : public DataSource<bool>
  {
      ConditionInterface* cond;
      mutable bool result;
  public:
      /**
       * DataSourceCondition takes ownership of the condition you pass
       * it.
       */
      DataSourceCondition( ConditionInterface* c );
      DataSourceCondition( const DataSourceCondition& orig );
      ~DataSourceCondition();
      bool get() const;
      bool value() const;
      void reset();
      ConditionInterface* condition() const;
      virtual DataSourceCondition* clone() const;
      virtual DataSourceCondition* copy( std::map<const DataSourceBase*, DataSourceBase*>& alreadyCloned ) const;
  };
}

#endif
