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
  \file api.hpp
  \brief API macros

  \author Mathias Soeken
*/

#pragma once

#include <string>
#include <vector>

#include <boost/hana/for_each.hpp>
#include <boost/hana/integral_constant.hpp>
#include <boost/hana/prepend.hpp>
#include <boost/hana/range.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>

#include <fmt/format.h>

#include "detail/python.hpp"

namespace alice
{

/* dynamic compile-time list construction based on https://stackoverflow.com/a/24092000 */
struct nil {};

template<typename T, typename U>
struct cons {};

template<typename List, typename Reversed>
struct list_reverse_helper;

template<typename Reversed>
struct list_reverse_helper<nil, Reversed> 
{
  using type = Reversed;
};

template<typename Head, typename Tail, typename Reversed>
struct list_reverse_helper<cons<Head, Tail>, Reversed>
{
  using type = typename list_reverse_helper<Tail, cons<Head, Reversed>>::type;
};

template<typename List>
struct list_to_tuple;

template<>
struct list_to_tuple<nil>
{
  static constexpr auto type = boost::hana::make_tuple();
};

template<typename Head, typename Tail>
struct list_to_tuple<cons<Head, Tail>>
{
  static constexpr auto type = boost::hana::prepend(list_to_tuple<Tail>::type, boost::hana::type_c<Head>);
};

template<typename T, int N>
struct list_maker_key : list_maker_key<T, N - 1> {};

template<typename T>
struct list_maker_key<T, 0> {};

#define _ALICE_START_LIST(name) \
  struct name##_list_maker {}; \
  static nil list_maker_helper_(list_maker_key<name##_list_maker, __COUNTER__>);

#define _ALICE_ADD_TO_LIST(name, type) \
  static cons<type, decltype(list_maker_helper_(list_maker_key<name##_list_maker, __COUNTER__>{}))> \
  list_maker_helper_(list_maker_key<name##_list_maker, __COUNTER__>);

#define _ALICE_END_LIST(name) \
  using name = typename list_reverse_helper<decltype(list_maker_helper_(list_maker_key<name##_list_maker, __COUNTER__>{})), nil>::type; 


#define ALICE_ADD_STORE(type, _option, _mnemonic, _name, _name_plural) \
template<> \
struct store_info<type> \
{ \
  static constexpr const char* key = #type; \
  static constexpr const char* option = _option; \
  static constexpr const char* mnemonic = _mnemonic; \
  static constexpr const char* name = _name; \
  static constexpr const char* name_plural = _name_plural; \
}; \
_ALICE_ADD_TO_LIST(alice_stores, type)

/* some global data structure */
struct alice_globals
{
  static alice_globals& get()
  {
    static alice_globals instance;
    return instance;
  }

  std::vector<std::pair<std::string, std::string>> command_names;
  std::vector<std::string> read_tags, write_tags;
  std::vector<std::string> read_names, write_names;
};

template<typename CLI, typename... Tags>
struct insert_read_commands
{
  /* constructor stores reference to CLI instance */
  insert_read_commands( CLI& cli ) : _cli( cli )
  {
    cli.set_category( "I/O" );
  }

  template<typename... Indexes>
  void operator()( Indexes... s )
  {
    []( ... ){}( apply<CLI, Tags>( _cli, s )... );
  }

private:
  template<typename CCLI, typename T>
  static int apply( CCLI& cli, std::size_t idx )
  {
    cli.template insert_read_command<T>( fmt::format( "read_{}", alice_globals::get().read_tags[idx] ), alice_globals::get().read_names[idx] );
    return 0;
  }

private:
  CLI& _cli;  
};

template<typename CLI, typename... Tags>
struct insert_write_commands
{
  /* constructor stores reference to CLI instance */
  insert_write_commands( CLI& cli ) : _cli( cli )
  {
    cli.set_category( "I/O" );
  }

  template<typename... Indexes>
  void operator()( Indexes... s )
  {
    []( ... ){}( apply<CLI, Tags>( _cli, s )... );
  }

private:
  template<typename CCLI, typename T>
  static int apply( CCLI& cli, std::size_t idx )
  {
    cli.template insert_write_command<T>( fmt::format( "write_{}", alice_globals::get().write_tags[idx] ), alice_globals::get().write_names[idx] );
    return 0;
  }

private:
  CLI& _cli;  
};

#define ALICE_DESCRIBE_STORE(type, element) \
template<> \
std::string to_string<type>( const type& element )

#define ALICE_PRINT_STORE(type, os, element) \
template<> \
void print<type>( std::ostream& os, const type& element )

#define ALICE_READ_FILE(type, tag, filename, cmd) \
template<> \
bool can_read<type, io_##tag##_tag_t>( command& cmd ) { return true; } \
template<> \
type read<type, io_##tag##_tag_t>( const std::string& filename, command& cmd )

#define ALICE_WRITE_FILE(type, tag, element, filename, cmd) \
template<> \
bool can_write<type, io_##tag##_tag_t>( command& cmd ) { return true; } \
template<> \
void write<type, io_##tag##_tag_t>( const type& element, const std::string& filename, command& cmd )

#define ALICE_ADD_FILE_TYPE(tag, name) \
struct io_##tag##_tag_t \
{ \
  io_##tag##_tag_t() \
  { \
    alice_globals::get().read_tags.push_back(#tag); \
    alice_globals::get().read_names.push_back(name); \
    alice_globals::get().write_tags.push_back(#tag); \
    alice_globals::get().write_names.push_back(name); \
  } \
}; \
io_##tag##_tag_t _##tag##_tag; \
_ALICE_ADD_TO_LIST(alice_read_tags, io_##tag##_tag_t) \
_ALICE_ADD_TO_LIST(alice_write_tags, io_##tag##_tag_t)

#define ALICE_ADD_FILE_TYPE_READ_ONLY(tag, name) \
struct io_##tag##_tag_t \
{ \
  io_##tag##_tag_t() \
  { \
    alice_globals::get().read_tags.push_back(#tag); \
    alice_globals::get().read_names.push_back(name); \
  } \
}; \
io_##tag##_tag_t _##tag##_tag; \
_ALICE_ADD_TO_LIST(alice_read_tags, io_##tag##_tag_t)

#define ALICE_ADD_FILE_TYPE_WRITE_ONLY(tag, name) \
struct io_##tag##_tag_t \
{ \
  io_##tag##_tag_t() \
  { \
    alice_globals::get().write_tags.push_back(#tag); \
    alice_globals::get().write_names.push_back(name); \
  } \
}; \
io_##tag##_tag_t _##tag##_tag; \
_ALICE_ADD_TO_LIST(alice_write_tags, io_##tag##_tag_t)

////////////////////////////////////////////////////////////////////////////////
// commands

template<typename CLI, typename... Cmds>
struct insert_commands
{
  /* constructor stores reference to CLI instance */
  insert_commands( CLI& cli ) : _cli( cli ) {}

  template<typename... Indexes>
  void operator()( Indexes... s )
  {
    []( ... ){}( apply<CLI, Cmds>( _cli, s )... );
  }

private:
  template<typename CCLI, typename T>
  static int apply( CCLI& cli, std::size_t idx )
  {
    cli.set_category( alice_globals::get().command_names[idx].second );
    cli.insert_command( alice_globals::get().command_names[idx].first, std::make_shared<T>( cli.env ) );
    return 0;
  }

private:
  CLI& _cli;  
};

#define _ALICE_COMMAND_INIT(name, category) \
struct name##_command_init \
{ \
  name##_command_init() \
  { \
    alice_globals::get().command_names.emplace_back(#name, category); \
  } \
}; \
name##_command_init _##name##_command_init;

#define ALICE_COMMAND(name, category, description) \
_ALICE_COMMAND_INIT(name, category) \
class name##_command; \
_ALICE_ADD_TO_LIST(alice_commands, name##_command) \
class name##_command : public command \
{ \
public: \
  name##_command( const environment::ptr& env ) : command( env, description ) {} \
protected: \
  void execute(); \
}; \
void name##_command::execute()

#define ALICE_ADD_COMMAND(name, category) \
_ALICE_COMMAND_INIT(name, category) \
_ALICE_ADD_TO_LIST(alice_commands, name##_command)

#define ALICE_INIT \
_ALICE_START_LIST( alice_stores ) \
_ALICE_START_LIST( alice_commands ) \
_ALICE_START_LIST( alice_read_tags ) \
_ALICE_START_LIST( alice_write_tags )

#define _ALICE_MAIN_BODY(prefix) \
  using namespace alice; \
  _ALICE_END_LIST( alice_stores ) \
  _ALICE_END_LIST( alice_commands ) \
  _ALICE_END_LIST( alice_read_tags ) \
  _ALICE_END_LIST( alice_write_tags ) \
  \
  using cli_t = decltype( boost::hana::unpack( list_to_tuple<alice_stores>::type, boost::hana::template_<alice::cli> ) )::type; \
  cli_t cli( #prefix ); \
  \
  constexpr auto rtags = list_to_tuple<alice_read_tags>::type; \
  constexpr auto rtags_with_cli = boost::hana::prepend( rtags, boost::hana::type_c<cli_t> ); \
  using insert_read_commands_t = decltype( boost::hana::unpack( rtags_with_cli, boost::hana::template_<insert_read_commands> ) )::type; \
  insert_read_commands_t irc( cli ); \
  boost::hana::unpack( boost::hana::make_range( boost::hana::size_c<0>, boost::hana::size( rtags ) ), irc ); \
  \
  constexpr auto wtags = list_to_tuple<alice_write_tags>::type; \
  constexpr auto wtags_with_cli = boost::hana::prepend( wtags, boost::hana::type_c<cli_t> ); \
  using insert_write_commands_t = decltype( boost::hana::unpack( wtags_with_cli, boost::hana::template_<insert_write_commands> ) )::type; \
  insert_write_commands_t iwc( cli ); \
  boost::hana::unpack( boost::hana::make_range( boost::hana::size_c<0>, boost::hana::size( wtags ) ), iwc ); \
  \
  constexpr auto ctags = list_to_tuple<alice_commands>::type; \
  constexpr auto ctags_with_cli = boost::hana::prepend( ctags, boost::hana::type_c<cli_t> ); \
  using insert_commands_t = decltype( boost::hana::unpack( ctags_with_cli, boost::hana::template_<insert_commands> ) )::type; \
  insert_commands_t ic( cli ); \
  boost::hana::unpack( boost::hana::make_range( boost::hana::size_c<0>, boost::hana::size( ctags ) ), ic );

#if defined ALICE_PYTHON
#define ALICE_MAIN(prefix) \
PYBIND11_MODULE(prefix, m) \
{ \
  _ALICE_MAIN_BODY(prefix) \
  alice::detail::create_python_module( cli, m ); \
}
#else
#define ALICE_MAIN(prefix) \
int main( int argc, char ** argv ) \
{ \
  _ALICE_MAIN_BODY(prefix) \
  return cli.run( argc, argv ); \
}
#endif

ALICE_INIT

}
