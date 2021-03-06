
// (C) Copyright Jonathan Turkanis and Tobias Schwinger
//
// Use modification and distribution are subject to the boost Software License,
// Version 1.0. (See http://www.boost.org/LICENSE_1_0.txt).

#ifndef BOOST_FT_FUNCTION_TYPE_PARAMETERS_HPP_INCLUDED
#define BOOST_FT_FUNCTION_TYPE_PARAMETERS_HPP_INCLUDED
//------------------------------------------------------------------------------
#include <boost/mpl/pop_front.hpp>
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include "function_type_signature.hpp"
//------------------------------------------------------------------------------
namespace boost {
//------------------------------------------------------------------------------
namespace function_types
{
  namespace detail
  {
    template<typename S,typename K> struct parameters_impl
    { };

    template<typename S> struct parameters_impl<S,static_function>
      : mpl::pop_front<S>
    { };

    template<typename S> struct parameters_impl<S,member_function>
      : mpl::pop_front< typename mpl::pop_front<S>::type >
    { };
  }
  template<typename T> struct function_type_parameters
    : detail::parameters_impl
      < typename function_type_signature<T>::types
      , typename detail::tag_core_type_id
        < typename function_type_signature<T>::kind >::type
      >
  { };
}
using function_types::function_type_parameters;
//------------------------------------------------------------------------------
} // namespace ::boost
//------------------------------------------------------------------------------
#endif // ndef BOOST_FT_FUNCTION_TYPE_PARAMETERS_HPP_INCLUDED
