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
  \file ps.hpp
  \brief Print statistics

  \author Mathias Soeken
*/

#pragma once

#include <vector>

#include <json.hpp>

#include "../command.hpp"

namespace alice
{

template<class... S>
class ps_command : public command
{
public:
  explicit ps_command( const environment::ptr& env )
    : command( env, "Print statistics" )
  {
    [](...){}( add_option_helper<S>( opts )... );
    add_flag( "--silent", "produce no output" );
  }

protected:
  rules validity_rules() const
  {
    return {
      {[this]() { (void)this; return any_true_helper<bool>( { is_set( store_info<S>::option )... } ); }, "no store has been specified" }
    };
  }

  void execute()
  {
    if ( !is_set( "silent" ) )
    {
      [](...){}( ps_store<S>()... );
    }
  }

  nlohmann::json log() const
  {
    nlohmann::json ret;
    [](...){}( ps_log_store<S>( ret )... );
    return ret;
  }

private:
  template<typename Store>
  int ps_store() const
  {
    constexpr auto option = store_info<Store>::option;
    constexpr auto name   = store_info<Store>::name;

    if ( is_set( option ) )
    {
      if ( store<Store>().current_index() == -1 )
      {
        env->out() << "[w] no " << name << " in store" << std::endl;
      }
      else
      {
        print_statistics<Store>( env->out(), store<Store>().current() );
      }
    }

    return 0;
  }

  template<typename Store>
  int ps_log_store( nlohmann::json& ret ) const
  {
    if ( !ret.empty() )
    {
      return 0;
    }

    constexpr auto option = store_info<Store>::option;

    if ( is_set( option ) )
    {
      if ( store<Store>().current_index() != -1 )
      {
        ret = log_statistics<Store>( store<Store>().current() );
      }
    }

    return 0;
  }
};

}
