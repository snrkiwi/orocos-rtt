/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:39 CET 2006  TypeStream.cxx

                        TypeStream.cxx -  description
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

#ifndef TYPESTREAM_IO_HPP
#define TYPESTREAM_IO_HPP 1

#include <iostream>

namespace RTT {
    // Code ripped from KDL 0.2


    // Eats space-like characters and comments
    // possibly returns the number of space-like characters eaten.
    extern int _EatSpace( std::istream& is,int* countp=NULL);

    // Eats whites, returns, tabs and the delim character
    //  Checks wether delim char. is encountered.
    extern bool Eat( std::istream& is, int delim );

    // Eats whites, returns, tabs and the delim character
    //  Checks wether delim char. is encountered.
    // EatEnd does not eat all space-like char's at the end.
    extern bool EatEnd( std::istream& is, int delim );
}

#endif
