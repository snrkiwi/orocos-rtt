/***************************************************************************
  tag: Peter Soetens  Thu Oct 10 16:19:04 CEST 2002  ProcessorInterface.hpp 

                        ProcessorInterface.hpp -  description
                           -------------------
    begin                : Thu October 10 2002
    copyright            : (C) 2002 Peter Soetens
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
 
#ifndef PROCESSORINTERFACE_HPP
#define PROCESSORINTERFACE_HPP

#include <string>

namespace ORO_CoreLib {
    class CommandInterface;
}


namespace ORO_Execution
{
    using ORO_CoreLib::CommandInterface;
    class StateMachineTree;
    class ProgramInterface;

    /**
     * @brief This abstract base class provides functionality to 
     * execute data on a configured system.	
     */
    class ProcessorInterface
    {
    public:
        	
        virtual ~ProcessorInterface()
        {}                            
            
        /**
         * Load a state context
         */
        virtual bool loadStateMachine(StateMachineTree* sc) = 0; 
			
        /**
         * Reset to the initial state
         */
        virtual bool resetStateMachine(const std::string& name) = 0;

        /**
         * Start evaluating a state context.
         */
        virtual bool startStateMachine(const std::string& name) = 0;

        /**
         * Stop evaluating a state context and go to the final state.
         */
        virtual bool stopStateMachine(const std::string& name) = 0;
            
        /**
         * Discard a state context.
         */
        virtual bool deleteStateMachine(const std::string& name) = 0;
            
        /**
         * Load a program to be executed.
         */
        virtual bool loadProgram( ProgramInterface* pi) = 0;
			
        /**
         * Start execution mode.
         */
        virtual bool startProgram(const std::string& name) = 0;

        /**
         * Start stepping/pause mode of a program.
         */
        virtual bool pauseProgram(const std::string& name) = 0;

        /**
         * Stop execution and reset logic to the beginning of the associated program.
         */
        virtual bool stopProgram(const std::string& name) = 0;
            
        /**
         * Discard the data currently loaded.
         */
        virtual bool deleteProgram(const std::string& name) = 0;

        /**
         * Return true if the Program has successfully started.
         */
        virtual bool isProgramRunning( const std::string& name) const = 0;

        /**
         * Return true if the StateMachine has successfully started.
         */
        virtual bool isStateMachineRunning( const std::string& name) const = 0;

        /**
         * Check if a StateMachine is in Stepped mode.
         * @return true if it is so, false if it is continuous.
         */
        virtual bool isStateMachineStepped(const std::string& name) const = 0;

        /**
         * Put a StateMachine in the state-at-a-time mode.
         */
        virtual bool steppedStateMachine(const std::string& name) = 0;

        /**
         * Put a StateMachine in the traverse-as-many-states-as-possible mode
         * (until no transition succeeds).
         */
        virtual bool continuousStateMachine(const std::string& name) = 0;

        /**
         * In stepping mode, execute one command of a program.
         */
        virtual bool stepProgram(const std::string& name) = 0;

        /**
         * Execute a given command asynchronously
         *
         * @param c The Command to be executed .
         * @return false if not possible or if
         * the previous command was not executed yet.
         */
        virtual bool process( CommandInterface* c) = 0;
    };

}

#endif 
