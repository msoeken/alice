/* alice: C++ command shell library
 * Copyright (C) 2017-2018  EPFL
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
  \file store.hpp
  \brief Store management

  \author Mathias Soeken
*/

#pragma once

#include <vector>

#include <fmt/format.h>

#include "../command.hpp"

namespace alice
{

template<typename S>
int show_helper( const command& cmd, const environment::ptr& env )
{
  constexpr auto option = store_info<S>::option;
  constexpr auto name_plural = store_info<S>::name_plural;

  const auto& store = env->store<S>();

  if ( cmd.is_set( option ) )
  {
    if ( store.empty() )
    {
      env->out() << fmt::format( "[i] no {} in store", name_plural ) << std::endl;
    }
    else
    {
      env->out() << fmt::format( "[i] {} in store:", name_plural ) << std::endl;
      auto index = 0;
      for ( const auto& element : store.data() )
      {
        env->out() << fmt::format( "  {} {:2}: ", ( store.current_index() == index ? '*' : ' ' ), index );
        env->out() << to_string<S>( element ) << std::endl;
        ++index;
      }
    }
  }

  return 0;
}

template<typename S>
int clear_helper( const command& cmd, const environment::ptr& env )
{
  constexpr auto option = store_info<S>::option;

  if ( cmd.is_set( option ) )
  {
    env->store<S>().clear();
  }
  return 0;
}

template<typename S>
int log_helper( const command& cmd, const environment::ptr& env, nlohmann::json& map )
{
  constexpr auto option = store_info<S>::option;

  if ( cmd.is_set( option ) )
  {
    map[option] = env->store<S>().current_index();
  }
  return 0;
}

template<class... S>
class store_command : public command
{
public:
  store_command( const environment::ptr& env )
      : command( env, "Store management" )
  {
    add_flag( "--show", "show contents" );
    add_flag( "--clear", "clear contents" );

    []( ... ) {}( add_option_helper<S>( opts )... );
  }

protected:
  rules_t validity_rules() const
  {
    return {
        {[this]() { return static_cast<unsigned>( is_set( "show" ) ) + static_cast<unsigned>( is_set( "clear" ) ) <= 1u; }, "only one operation can be specified"},
        {[this]() { return any_true_helper<bool>( {is_set( store_info<S>::option )...} ); }, "no store has been specified"}};
  }

  void execute()
  {
    if ( is_set( "show" ) || !is_set( "clear" ) )
    {
      []( ... ) {}( show_helper<S>( *this, env )... );
    }
    else if ( is_set( "clear" ) )
    {
      []( ... ) {}( clear_helper<S>( *this, env )... );
    }
  }

  nlohmann::json log() const
  {
    nlohmann::json map;
    []( ... ) {}( log_helper<S>( *this, env, map )... );
    return map;
  }
};
}
