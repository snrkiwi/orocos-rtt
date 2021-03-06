'/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:20 CET 2004  SimpleDemarshaller.hpp

                        SimpleDemarshaller.hpp -  description
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

#ifndef PI_PROPERTIES_SIMPLE_DEMARSHALLER
#define PI_PROPERTIES_SIMPLE_DEMARSHALLER

#include "../Property.hpp"

#include <vector>
#include <map>
#include <iostream>
#include <fstream>

namespace RTT
{

	/**
	 * A simple demarshaller.
	 *
	 * @see SimpleMarshaller
	 */
    class RTT_API SimpleDemarshaller : public Demarshaller
    {
      static const char TYPECODE_BOOL = 'B';
      static const char TYPECODE_CHAR = 'C';
      static const char TYPECODE_INT = 'I';
      static const char TYPECODE_DOUBLE = 'D';
      static const char TYPECODE_STRING = 'S';

public:
      typedef std::istream input_stream;

    SimpleDemarshaller(const std::string& filename)
    {
      // FIXME first check if the target file exists...
      _is = new std::ifstream( filename.c_str() );
    }

            virtual bool deserialize(PropertyBag &v)
			{
				using namespace std;
#if 0
				char begin_token, end_token;
      *_is >> begin_token;
				if(begin_token == '{')
					cerr << "read begin";
#endif

				char c;
      while( _is->read(&c,1) )
				{
					unsigned char name_length, value_length;
					char name[256];
					char value[256];
					int intvalue;
					double doublevalue;
	  _is->read(reinterpret_cast<char*>(&name_length),1);
	  _is->read(name,name_length);
					name[name_length] = 0;
	  _is->ignore();
	  _is->read(reinterpret_cast<char*>(&value_length),1);
	  _is->read(value,value_length);
					switch(c)
					{
						case TYPECODE_BOOL:
							if(value[0])
								v.add(new Property<bool>(name,"",false));
							else
								v.add(new Property<bool>(name,"",true));
							cerr << "bool: ";
	      _is->ignore();
							break;
						case TYPECODE_CHAR:
							v.add(new Property<char>(name,"",value[0]));
							cerr << "char: ";
	      _is->ignore();
							break;
						case TYPECODE_INT:
							value[value_length]=0;
							sscanf(value, "%d", &intvalue);
							v.add(new Property<int>(name,"",intvalue));
							cerr << "int: ";
	      _is->ignore();
							break;
						case TYPECODE_DOUBLE:
							value[value_length]=0;
							sscanf(value, "%lf", &doublevalue);
							v.add(new Property<double>(name,"",doublevalue));
							cerr << "double: ";
	      _is->ignore();
							break;
						case TYPECODE_STRING:
							value[value_length]=0;
							cerr << value<<"\n";
							v.add(new Property<string>(name,"",value));
	      _is->ignore();
							cerr << "string: ";
							break;
                        default:
							cerr << "lost synchro" << endl;
					}
					cerr << "\n"<<(int)name_length<<"\n"<<name<<"\n"<<(int)value_length<<endl;
				}
#if 0
                for (
                    vector<PropertyBase*>::iterator i = v._properties.begin();
                    i != v._properties.end();
                    i++ )
                {
                    (*i)->serialize(*this);
                }
#endif
//                _os <<"</Bag>\n";

#if 0
      *_is >> end_token;
				if(end_token == '}')
					cerr << "read end";
#endif
                return true;
			}

  protected:
    // Input stream
    input_stream * _is;


    };
}
#endif
