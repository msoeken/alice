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

/*
  \file api.hpp
  \brief API macros

  \author Mathias Soeken
*/

#pragma once

#include <boost/hana/prepend.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>

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

#define ALICE_DESCRIBE_STORE(type, element) \
template<> \
std::string to_string<std::string>( const type& element )

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

#define ALICE_ADD_FILE_TYPE(tag) \
struct io_##tag##_tag_t;

#define ALICE_INIT _ALICE_START_LIST(alice_stores)

}