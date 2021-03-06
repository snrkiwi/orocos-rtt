/***************************************************************************
  tag: Peter Soetens  Tue Apr 5 16:53:25 CEST 2005  NonPeriodicActivity.cxx

                        NonPeriodicActivity.cxx -  description
                           -------------------
    begin                : Tue April 05 2005
    copyright            : (C) 2005 Peter Soetens
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


#ifdef ORO_PRAGMA_INTERFACE
#pragma implementation
#endif
#include "NonPeriodicActivity.hpp"
#include "Logger.hpp"

namespace RTT
{
    NonPeriodicActivity::NonPeriodicActivity(int priority, RunnableInterface* _r )
        : ActivityInterface(_r),
        OS::SingleThread(priority, "NonPeriodicActivity" )
    {
    }

    NonPeriodicActivity::NonPeriodicActivity(int scheduler, int priority, RunnableInterface* _r )
        : ActivityInterface(_r),
        OS::SingleThread(scheduler, priority, "NonPeriodicActivity" )
    {
    }

    NonPeriodicActivity::NonPeriodicActivity(int priority, const std::string& name, RunnableInterface* _r )
        : ActivityInterface(_r),
        OS::SingleThread(priority, name )
    {
    }

    NonPeriodicActivity::~NonPeriodicActivity()
    {
        this->stop();
    }

    Seconds NonPeriodicActivity::getPeriod() const { return 0; }

    OS::ThreadInterface* NonPeriodicActivity::thread() { return this; }

    bool NonPeriodicActivity::isPeriodic() const { return false; }

    bool NonPeriodicActivity::initialize() {
        if ( runner )
            return runner->initialize();
        return true;
    }

    void NonPeriodicActivity::loop() {
        if ( runner )
            runner->loop();
    }

    bool NonPeriodicActivity::breakLoop() {
        if ( runner )
            return runner->breakLoop();
        return false;
    }

    void NonPeriodicActivity::finalize() {
        if ( runner )
            runner->finalize();
    }


    bool NonPeriodicActivity::start() {
        return SingleThread::start();
    }

    bool NonPeriodicActivity::stop() {
        return SingleThread::stop();
    }

    bool NonPeriodicActivity::trigger() {
        return SingleThread::isActive() ? SingleThread::start() : false;
    }

    bool NonPeriodicActivity::execute() {
        return false;
    }

    bool NonPeriodicActivity::isRunning() const {
        return SingleThread::isRunning();
    }

    bool NonPeriodicActivity::isActive() const {
        return SingleThread::isActive();
    }



}
