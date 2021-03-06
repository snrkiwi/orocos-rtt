/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:38 CET 2006  CommandDataSource.hpp

                        CommandDataSource.hpp -  description
                           -------------------
    begin                : Wed January 18 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@mech.kuleuven.be

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


#ifndef ORO_COMMAND_DATASOURCE_HPP
#define ORO_COMMAND_DATASOURCE_HPP

#include "DataSource.hpp"
#include "CommandInterface.hpp"

namespace RTT
{
    /**
     * A Command which evaluates a DataSourceBase and always returns true.
     */
    struct CommandDataSource :
        public CommandInterface
    {
        DataSourceBase::shared_ptr _dsb;
        CommandDataSource( DataSourceBase::shared_ptr dsb )
            : _dsb(dsb) {}

        void readArguments()
        {
            _dsb->evaluate();
        }

        bool execute() {
            return true;
        }

        void reset() {
            _dsb->reset();
        }

        CommandInterface* clone() const {
            return new CommandDataSource( _dsb );
        }

        CommandInterface* copy( std::map<const DataSourceBase*, DataSourceBase*>& alreadyCloned ) const {
            return new CommandDataSource( _dsb->copy( alreadyCloned ) );
        }

    };

    /**
     * A Command which evaluates a DataSource<bool> and returns the result of get().
     */
    struct CommandDataSourceBool :
        public CommandInterface
    {
        DataSource<bool>::shared_ptr _dsb;
        CommandDataSourceBool( DataSource<bool>::shared_ptr dsb )
            : _dsb(dsb) {}

        void readArguments()
        {
            _dsb->evaluate();
        }

        bool execute() {
            return _dsb->value();
        }
        void reset() {
            _dsb->reset();
        }
        CommandInterface* clone() const {
            return new CommandDataSourceBool( _dsb );
        }

        CommandInterface* copy( std::map<const DataSourceBase*, DataSourceBase*>& alreadyCloned ) const {
            return new CommandDataSourceBool( _dsb->copy( alreadyCloned ) );
        }

    };

}

#endif
