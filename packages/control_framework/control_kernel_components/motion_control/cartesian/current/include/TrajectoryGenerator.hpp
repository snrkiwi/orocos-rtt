/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:21 CET 2004  TrajectoryGenerator.hpp 

                        TrajectoryGenerator.hpp -  description
                           -------------------
    begin                : Mon January 19 2004
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
 
#ifndef TRAJECTORYGENERATOR_HPP
#define TRAJECTORYGENERATOR_HPP

#include <geometry/trajectory.h>
#include <geometry/frames.h>
#include <corelib/PropertyComposition.hpp>

namespace ORO_ControlKernel
{
    using namespace ORO_Geometry;
#if 0
    /**
     * @brief This Generator reads a Trajectory in the Task Frame and converts
     * it to the robot frame. It can read a Trajectory from a file or
     * can use an existing Trajectory object.
     *
     * It will only read a new trajectory when the previous is done.
     * 
     * @warning This code is experimental an most likely does not compile.
     *
     * @todo TODO Finish this implementation.
     * @deprecated
     * @see CartesianGenerator
     */
    template< class Base>
    class TrajectoryGenerator
        : public Base
    {
        public:
        typedef typename Base::Command Commands;
        typedef typename Base::SetPoint SetPoints;
        typedef typename Base::SetPointType SetPointType;
            TrajectoryGenerator() 
                : filename("FileName","Traject File Name"), 
                  repeat("CyclicTrajectory","True if current trajectory needs to be cyclicly repeated", false),
                  refreshFile("RefreshTrajectory", "True if trajectory needs to be re-read from file in each startup", true),
                  acceptCommands("AcceptCommands", "True if the Generator accepts trajectories through the Commands Data Object", true),
                  task_base_frame("TaskRobotFrame", "The Task Frame expressed in the Robot Base Frame"),
                  robot_world_frame("RobotWorldFrame", "The Robot Frame expressed in the World Frame"),
                  task_world_frame("TaskWorldFrame", "The Task Frame expressed in the World Frame"),
                  home_task_frame("HomeTaskFrame", "The Home Position of the Robot in the Task Frame"),
                  tr(0), time_stamp(0), time(0), run(false)
            {}

            void pull()
            {
                if (!run)
                    return;

                if ( acceptCommands && !repeat && trajectDone() )
                {
                    // Read new traject when done with old.
                    typename Base::CommandType new_com;
                    Commands::dObj()->Get(new_com);
                    tr = new_com.trajectory;
                    time_stamp = TimeService::Instance()->getTicks();
                } else 
                    if (repeat && trajectDone() )
                {
                    // Repeat current traject (dangerous !)
                    time_stamp = TimeService::Instance()->getTicks();
                }

                time = TimeService::Instance()->getSeconds(time_stamp);
            }

            void calculate()
            {
                if (run)
                {
                    fr = task_base_frame.get() * tr->Pos( time );
                    tw = task_base_frame.get() * tr->Vel( time );
                }
            }

            void push()
            {
                result.mp_base_frame = fr;
                result.mp_base_twist = tw;
                SetPoints::dObj()->Set(result);
            }

            bool updateProperties(PropertyBag& bag)
            {
                if (refreshFile)
                    if ( composeProperty(bag, filename) )
                    {
                        std::ifstream f( filename.get().c_str() );
                        tr = Trajectory::Read(f);
                        composeProperty(bag, repeat);
                        composeProperty(bag, acceptCommands);
                        composeProperty(bag, repeat);
                        if (!composeProperty(bag, refreshFile))
                            refreshFile = false;
                        return true;
                        fr = tr->Pos(0);
                        tw = tr->Vel(0);
                    }
                return false;
            }

            void setTaskInBaseFrame( const Frame& f )
            {
                task_base_frame = f;
                task_world_frame = robot_world_frame * task_base_frame;
            }

            void setTaskInWorldFrame( const Frame& f )
            {
                task_world_frame = f;
                task_base_frame  = robot_world_frame.get().Inverse() * task_world_frame;
            }

            void setRobotInWorldFrame( const Frame& f )
            {
                robot_world_frame = f;
                task_base_frame  = robot_world_frame.get().Inverse() * task_world_frame;
            }

            void resetTime( double newTime = 0)
            {
                time_stamp = TimeService::Instance()->getTicks() - TimeService::nsecs2ticks(newTime);
            }

            /**
             * Scales the execution time of the traject with X percent.
             * The scaling will be limited with the current trajectory's
             * limits. 
             * 
             * @param percent A percentage inidicating the relative new trajectory duration.
             */
            void timeScale( double percent )
            {
                VelocityProfile* t_prof = tr->GetProfile();
                double oldDur = t_prof->Duration();
                double newDur = percent / 100.0 * oldDur;
                
                t_prof->SetProfileDuration(t_prof->Pos(0), t_prof->Pos(oldDur), newDur);
                
                double fact  = t_prof->Duration() / oldDur;
                time *= fact;
                resetTime( time );
            }

            void start()
            {
                if (run)
                    return;
                
                if (acceptCommands)
                    tr = Commands::dObj->get().trajectory;
                if (tr)
                    run = true;
                
                time_stamp = TimeService::Instance()->getTicks();
            }

            void stop()
            {
                run = false;
            }

            bool trajectDone()
            {
                if (tr)
                    return time > tr->Duration();
                return false;
            }

            bool setTrajectory(ORO_Geometry::Trajectory* _t)
            {
                if (!run)
                {
                    tr = _t;
                    return true;
                }
                return false;
            }
            
        protected:
            Property<std::string> filename;
            Property<bool>        repeat;
            Property<bool>        refreshFile;
            Property<bool>        acceptCommands;
            Property<Frame>       task_base_frame;
            Property<Frame>       robot_world_frame;
            Property<Frame>       task_world_frame;
            Property<Frame>       home_task_frame;

            SetPointType result;
            Frame fr;
            Twist tw;
            const Trajectory* tr;
            TimeService::ticks time_stamp;
            double time;
            bool run;
    };
#endif
}
#endif