/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:25 CET 2004  ExpressionParser.hpp

                        ExpressionParser.hpp -  description
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

#ifndef EXPRESSIONPARSER_HPP
#define EXPRESSIONPARSER_HPP

#include "parser-types.hpp"
#include "CommonParser.hpp"
#include "PeerParser.hpp"
#include "ValueParser.hpp"
#include "../DataSource.hpp"
#include "../Operators.hpp"
#include "../Time.hpp"

#include <stack>

#ifdef ORO_PRAGMA_INTERFACE
#pragma interface
#endif

namespace RTT { namespace detail
{
  /**
   * This parser parses a call of the form
   * "a.b( arg1, arg2, ..., argN )".
   *
   * @todo check why lexeme_d[] is used in implementation,
   * thus why datacalls are parsed on the character level
   * instead of on the phrase level. (probably for the dots ?)
   */
  class DataCallParser
  {
    DataSourceBase::shared_ptr ret;
    std::string mobject;
    std::string mmethod;

    rule_t datacall, arguments;

    void seenmethodname( iter_t begin, iter_t end )
      {
        std::string name( begin, end );
        mmethod = name;
      };

    void seendataname();
    void seendatacall();
    CommonParser& commonparser;
    ExpressionParser& expressionparser;
    PeerParser peerparser;
    std::stack<ArgumentsParser*> argparsers;
  public:
    DataCallParser( ExpressionParser& p, CommonParser& cp, TaskContext* pc );
    ~DataCallParser();

    rule_t& parser()
      {
        return datacall;
      };

    DataSourceBase* getParseResult()
      {
        return ret.get();
      };
  };

  /**
   * How we parse:  this parser works like a stack-based RPN
   * calculator.  An atomic expression pushes one DataSource up the
   * stack, a binary expression pops two DataSources, and pushes a new
   * one, a unary pops one, and pushes one etc.  This allows for the
   * reentrancy we need..
   */
  class ExpressionParser
  {
    rule_t expression, unarynotexp, unaryminusexp, unaryplusexp, div_or_mul,
      modexp, plusexp, minusexp, smallereqexp, smallerexp,
      greatereqexp, greaterexp, equalexp, notequalexp, orexp, andexp,
      ifthenelseexp, dotexp, groupexp, atomicexpression,
      time_expression, time_spec, indexexp, comma, open_brace, close_brace;

    /**
     * The parse stack..  see the comment for this class ( scroll up
     * ;) ) for info on the general idea.
     * We keep a reference to the DataSources in here, while they're
     * in here..
     */
    std::stack<DataSourceBase::shared_ptr> parsestack;

    // the name that was parsed as the object to use a certain
    // data of..
    std::string mobjectname;

    // the name that was parsed as the name of the data to use
    // from the object with name mobjectname.
    std::string mpropname;

    // time specification
    nsecs tsecs;

    void seen_unary( const std::string& op );
    void seen_binary( const std::string& op );
    void seen_dotmember( iter_t begin, iter_t end );
    void seenvalue();
    void seendatacall();
    void seentimespec( int n );
    void seentimeunit( iter_t begin, iter_t end );
      void inverttime();
      void seentimeexpr();

      DataCallParser datacallparser;
      /**
       * The governing common parser.
       */
      CommonParser& commonparser;
      ValueParser valueparser;
      bool _invert_time;
      OperatorRepository::shared_ptr opreg;
  public:
      ExpressionParser( TaskContext* pc, CommonParser& common_parser );
    ~ExpressionParser();

    rule_t& parser();

    DataSourceBase::shared_ptr getResult();
    // after an expression is parsed, the resultant DataSourceBase will
    // still be on top of the stack, and it should be removed before
    // going back down the parse stack.  This is what this function
    // does..
    void dropResult();

      bool hasResult() { return !parsestack.empty(); }
  };
}}

#endif
