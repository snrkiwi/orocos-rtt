/***************************************************************************
  tag: FMTC  Tue Mar 11 21:49:25 CET 2008  Timer.cpp

                        Timer.cpp -  description
                           -------------------
    begin                : Tue March 11 2008
    copyright            : (C) 2008 FMTC
    email                : peter.soetens@fmtc.be

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


#include "Timer.hpp"
#include "os/MutexLock.hpp"
#include "os/SingleThread.hpp"
#include "Logger.hpp"

namespace RTT
{

    bool Timer::initialize() {
        // only start if non periodic.
        return this->getThread()->getPeriod() == 0;
    }
    void Timer::finalize() {}

    void Timer::step() {
        // no implementation for periodic execution.
    }

    void Timer::loop()
    {
        // This code is executed from mThread's thread
        while (!mdo_quit) {
            Time    wake_up_time;
            TimerId next_timer_id = 0;
            bool    have_timer = false;

            // Select next timer.
            {// This scope is for OS::MutexLock.
                // find wake_up_time
                // check timers queue.
                OS::MutexLock locker(m);
                /// \todo InfiniteNSecs should probably be clock-specific
                wake_up_time = TimeService::InfiniteNSecs;
                for (TimerIds::iterator it = mtimers.begin(); it != mtimers.end(); ++it) {
                    if ( (it->first != 0) && (it->first < wake_up_time) ) {
                        wake_up_time = it->first;
                        next_timer_id = it - mtimers.begin();
                        have_timer = true;
                    }
                }
            }// OS::MutexLock

            if (have_timer)
            {
                bool	did_timeout = false;
                if ( wake_up_time > mTimeserv->getNSecs() )
                {
                    // now convert into wall clock time, as mTimeserv is not
                    // guaranteed to be a wall clock!
                    Time wake_up_time_wall = mTimeserv->toWallClock(wake_up_time);
                    did_timeout = !msem.waitUntil( wake_up_time_wall );
                }
                else
                {
                    did_timeout = true;	// have passed the time!
                }

                // Timeout handling
                if (did_timeout)
                {
                    // First: reset/reprogram the timer that expired:
                    {
                        OS::MutexLock locker(m);
                        // detect corner case for resize:
                        if ( next_timer_id < int(mtimers.size()) ) {
                            // now clear or reprogram it.
                            TimerIds::iterator tim = mtimers.begin() + next_timer_id;
                            if ( tim->second ) {
                                // periodic timer
                                tim->first += tim->second;
                            } else {
                                // aperiodic timer
                                tim->first = 0;
                            }
                        }
                    }// OS::MutexLock

                    // Second: send the timeout signal and allow (within the callback)
                    // to reprogram the timer.
                    // If we would first call timeout(), the code above would overwrite
                    // user settings.
                    timeout( next_timer_id );
                }
                // else we were interrupted, so go around again
            }
            else
            {
                // Sleep for an abritrary amount of time, to ease CPU burden.
                // Any user activity on the timers will cause us to wake.
                (void)msem.waitUntil( 
                    /// \todo update when TimeService supports non-static getNSecs()
                    nsecs_to_Seconds(RTT::TimeService::Instance()->getNSecs())
                    + (60. * 60.));   // 1 hour
            }
        }
    }

    bool Timer::breakLoop()
    {
        mdo_quit = true;
        msem.signal();
        return true;
    }

    Timer::Timer(TimerId max_timers, int scheduler, int priority)
        : mThread(0), msem(0), mdo_quit(false)
    {
        mTimeserv = TimeService::Instance();
        mtimers.resize(max_timers);
        if (scheduler != -1) {
            mThread = new OS::SingleThread(scheduler, priority, "Timer", this);
            mThread->start();
        }
    }

    Timer::Timer(TimeService* clock, TimerId max_timers, int scheduler, int priority)
            : mTimeserv(clock), mThread(0), msem(0), mdo_quit(false)
    {
        assert(mTimeserv);
        mtimers.resize(max_timers);
        if (scheduler != -1) {
            mThread = new OS::SingleThread(scheduler, priority, "Timer", this);
            mThread->start();
        }
    }

    Timer::~Timer()
    {
        delete mThread;
    }


    void Timer::timeout(TimerId timer_id)
    {
        // User must implement this method.
    }

    void Timer::setMaxTimers(TimerId max)
    {
        OS::MutexLock locker(m);
        mtimers.resize(max, std::make_pair(Time(0), Time(0)) );
    }

    bool Timer::startTimer(TimerId timer_id, double period)
    {
        if ( timer_id < 0 || timer_id > int(mtimers.size()) || period < 0.0)
        {
            log(Error) << "Invalid timer id or period" << endlog();
            return false;
        }

        Time due_time = mTimeserv->getNSecs() + Seconds_to_nsecs( period );

        {
            OS::MutexLock locker(m);
            mtimers[timer_id].first = due_time;
            mtimers[timer_id].second = Seconds_to_nsecs( period );
        }
        msem.signal();
        return true;
    }

    bool Timer::arm(TimerId timer_id, double wait_time)
    {
        if ( timer_id < 0 || timer_id > int(mtimers.size()) || wait_time < 0.0)
        {
            log(Error) << "Invalid timer id or wait time" << endlog();
            return false;
        }

        Time now = mTimeserv->getNSecs();
        Time due_time = now + Seconds_to_nsecs( wait_time );

        {
            OS::MutexLock locker(m);
            mtimers[timer_id].first  = due_time;
            mtimers[timer_id].second = 0;
        }
        msem.signal();
        return true;
    }

    bool Timer::isArmed(TimerId timer_id) const
    {
        OS::MutexLock locker(m);
        if (timer_id < 0 || timer_id > int(mtimers.size()) )
        {
            log(Error) << "Invalid timer id" << endlog();
            return false;
        }
        return mtimers[timer_id].first != 0;
    }

    double Timer::timeRemaining(TimerId timer_id) const
    {
        OS::MutexLock locker(m);
        if (timer_id < 0 || timer_id > int(mtimers.size()) )
        {
            log(Error) << "Invalid timer id" << endlog();
            return 0.0;
        }
        Time now = mTimeserv->getNSecs();
        Time result = mtimers[timer_id].first - now;
        // detect corner cases.
        if ( result < 0 )
            return 0.0;
        return nsecs_to_Seconds( result );
    }

    bool Timer::killTimer(TimerId timer_id)
    {
        OS::MutexLock locker(m);
        if (timer_id < 0 || timer_id > int(mtimers.size()) )
        {
            log(Error) << "Invalid timer id" << endlog();
            return false;
        }
        mtimers[timer_id].first = 0;
        mtimers[timer_id].second = 0;
        return true;
    }



}
