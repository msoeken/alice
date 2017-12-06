/* alice: C++ command shell library
 * Copyright (C) 2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*
  \file python.hpp
  \brief Python interface

  \author Mathias Soeken
*/

#pragma once

#if defined ALICE_PYTHON

#include <string>

#include <fmt/format.h>
#include <pybind11/pybind11.h>

#include "../command.hpp"

namespace py = pybind11;

namespace alice
{

namespace detail
{

struct log_var_python_visitor
{
public:
  py::object operator()( const std::string& s ) const
  {
    return py::str( s );
  }

  py::object operator()( int i ) const
  {
    return py::int_( i );
  }

  py::object operator()( unsigned i ) const
  {
    return py::int_( static_cast<int>( i ) );
  }

  py::object operator()( uint64_t i ) const
  {
    return py::int_( i );
  }

  py::object operator()( double d ) const
  {
    return py::float_( d );
  }

  py::object operator()( bool b ) const
  {
    return py::bool_( b );
  }

  template<typename T>
  py::object operator()( const std::vector<T>& v ) const
  {
    py::list value;

    for ( const auto& element : v )
    {
      value.append( operator()( element ) );
    }

    return value;
  }
};

class return_value_dict
{
public:
  return_value_dict( const command::log_map_t& map )
  {
    log_var_python_visitor vis;

    for ( const auto& lp : map )
    {
      if ( lp.first == "__repr__" )
      {
        repr = mpark::get<std::string>( lp.second );
      }
      else if ( lp.first == "_repr_html_" )
      {
        repr_html = mpark::get<std::string>( lp.second );
      }
      else
      {
        const auto value = mpark::visit( vis, lp.second );
        _dict[py::str( lp.first )] = value;
      }
    }
  }

  py::object __getitem__( const std::string& key ) const
  {
    return _dict.attr( "__getitem__" )( py::str( key ) );
  }

  std::string __repr__() const
  {
    if ( repr.empty() )
    {
      return py::str( _dict.attr( "__repr__" )() );
    }
    else
    {
      return repr;
    }
  }

  std::string _repr_html_() const
  {
    if ( repr_html.empty() )
    {
      return fmt::format( "<pre>{}</pre>", __repr__() );
    }
    else
    {
      return repr_html;
    }
  }

  const py::dict& dict() const
  {
    return _dict;
  }

private:
  py::dict    _dict;

  std::string repr;
  std::string repr_html;
};

template<typename CLI>
void create_python_module( const CLI& cli, py::module& m )
{
  m.doc() = "Python bindings";

  py::class_<return_value_dict> representer( m, "ReturnValueDict" );
  representer
    .def( "__getitem__", &return_value_dict::__getitem__ )
    .def( "__repr__", &return_value_dict::__repr__ )
    .def( "_repr_html_", &return_value_dict::_repr_html_ )
    .def( "dict", &return_value_dict::dict )
    ;
  
  for ( const auto& p : cli.env->commands )
  {
    m.def( p.first.c_str(), [p]( py::kwargs kwargs ) -> py::object {
      std::vector<std::string> pargs = {p.first};

      for ( const auto& kp : kwargs )
      {
        // get the key as string
        const auto skey = kp.first.cast<std::string>();
        const auto value = kp.second;

        // TODO cast float to string?
        if ( py::isinstance<py::bool_>( value ) )
        {
          if ( value.cast<bool>() )
          {
            pargs.push_back( "--" + skey );
          }
        }
        else if ( py::isinstance<py::int_>( value ) )
        {
          pargs.push_back( "--" + skey );
          pargs.push_back( std::to_string( value.cast<int>() ) );
        }
        else
        {
          pargs.push_back( "--" + skey );
          pargs.push_back( value.cast<std::string>() );
        }
      }
      p.second->run( pargs );

      const auto log = p.second->log();

      if ( log )
      {
        return py::cast( return_value_dict( *log ) );
      }
      else
      {
        return py::none();
      }
    }, p.second->caption().c_str() );
  }
}

}

}

#endif
