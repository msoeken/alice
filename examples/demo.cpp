/* alice: C++ REPL library
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

#include <alice/alice.hpp>

#include <fstream>
#include <sstream>
#include <string>

#include <fmt/format.h>

namespace alice
{

/* setup string store */
template<>
struct store_info<std::string>
{
  static constexpr const char* key = "string";
  static constexpr const char* option = "str";
  static constexpr const char* mnemonic = "s";
  static constexpr const char* name = "String";
  static constexpr const char* name_plural = "Strings";
};

struct io_text_tag_t;

template<>
std::string to_string<std::string>( const std::string& element )
{
  return fmt::format( "{} characters", element.size() );
}

template<>
void print<std::string>( std::ostream& out, const std::string& element )
{
  out << element << std::endl;
}

template<>
bool can_read<std::string, io_text_tag_t>( command& ) { return true; }

template<>
std::string read<std::string, io_text_tag_t>( const std::string& filename, command& )
{
  std::ifstream in( filename.c_str(), std::ifstream::in );
  std::stringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

template<>
bool can_write<std::string, io_text_tag_t>( command& ) { return true; }

template<>
void write<std::string, io_text_tag_t>( const std::string& element, const std::string& filename, command& )
{
  std::ofstream out( filename.c_str(), std::ofstream::out );
  out << element;
}
}

int main( int argc, char** argv )
{
  alice::cli<std::string> cli( "demo" );

  cli.set_category( "I/O" );
  cli.insert_read_command<alice::io_text_tag_t>( "read_text", "Text" );
  cli.insert_write_command<alice::io_text_tag_t>( "write_text", "Text" );

  return cli.run( argc, argv );
}
