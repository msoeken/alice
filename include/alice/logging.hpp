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
  \file logging.hpp
  \brief Data structures for logging

  \author Mathias Soeken
*/

#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <mpark/variant.hpp>

namespace alice
{

namespace detail
{
inline std::string json_escape( const std::string& s )
{
  std::stringstream ss;

  for ( size_t i = 0; i < s.length(); ++i )
  {
    if ( s[i] == '\\' || s[i] == '"' )
    {
      ss << "\\" << s[i];
    }
    else if ( unsigned( s[i] ) < '\x20' )
    {
      ss << "\\u" << std::setfill( '0' ) << std::setw( 4 ) << std::hex << unsigned( s[i] );
    }
    else
    {
      ss << s[i];
    }
  }

  return ss.str();
}

class log_var_visitor
{
public:
  log_var_visitor( std::ostream& os ) : os( os ) {}

  void operator()( const std::string& s ) const
  {
    os << "\"" << json_escape( s ) << "\"";
  }

  void operator()( int i ) const
  {
    os << i;
  }

  void operator()( unsigned i ) const
  {
    os << i;
  }

  void operator()( uint64_t i ) const
  {
    os << i;
  }

  void operator()( double d ) const
  {
    os << d;
  }

  void operator()( bool b ) const
  {
    os << ( b ? "true" : "false" );
  }

  template<typename T>
  void operator()( const std::vector<T>& v ) const
  {
    os << "[";

    bool first = true;

    for ( const auto& element : v )
    {
      if ( !first )
      {
        os << ", ";
      }
      else
      {
        first = false;
      }

      operator()( element );
    }

    os << "]";
  }

private:
  std::ostream& os;
};
}

class logger
{
public:
  using log_var_t = mpark::variant<std::string, int, unsigned, uint64_t, double, bool, std::vector<std::string>, std::vector<int>, std::vector<unsigned>, std::vector<uint64_t>, std::vector<std::vector<int>>, std::vector<std::vector<unsigned>>>;
  using log_map_t = std::unordered_map<std::string, log_var_t>;
  using log_opt_t = std::shared_ptr<log_map_t>;

public:
  void start( const std::string& filename )
  {
    logf.open( filename.c_str(), std::ofstream::out );
    logf << "[";
  }

  void log( const log_opt_t& cmdlog, const std::string& cmdstring, const std::chrono::system_clock::time_point& start )
  {
    if ( !log_first_command )
    {
      logf << "," << std::endl;
    }
    else
    {
      log_first_command = false;
    }

    const auto start_c = std::chrono::system_clock::to_time_t( start );
    char timestr[20];
    std::strftime( timestr, sizeof( timestr ), "%F %T", std::localtime( &start_c ) );
    logf << fmt::format( "{{\n"
                         "  \"command\": \"{}\",\n"
                         "  \"time\": \"{}\"",
                         detail::json_escape( cmdstring ), timestr );

    if ( cmdlog )
    {
      detail::log_var_visitor vis( logf );

      for ( const auto& p : *cmdlog )
      {
        logf << fmt::format( ",\n  \"{}\": ", p.first );
        mpark::visit( vis, p.second );
      }
    }

    logf << "\n}";
  }

  void stop()
  {
    logf << "]" << std::endl;
  }

private:
  std::ofstream logf;
  bool log_first_command{true};
};
}
