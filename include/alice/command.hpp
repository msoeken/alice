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
  \file command.hpp
  \brief Data structures for command and environment

  \author Mathias Soeken
*/

#pragma once

#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <CLI/CLI.hpp>
#include <fmt/format.h>

#include "logging.hpp"
#include "store.hpp"
#include "store_api.hpp"

namespace alice
{

class command;

template<class... S>
class cli;

class environment
{
public:
  using ptr = std::shared_ptr<environment>;

  /*! \brief Adds store to environment */
  template<typename T>
  void add_store()
  {
    constexpr auto key = store_info<T>::key;
    constexpr auto name = store_info<T>::name;

    stores.emplace( key, std::shared_ptr<void>( new alice::store<T>( name ) ) );
  }

  /*! \brief Retrieves store from environment */
  template<typename T>
  store<T>& store() const
  {
    constexpr auto key = store_info<T>::key;

    return *( reinterpret_cast<alice::store<T>*>( stores.at( key ).get() ) );
  }

  /*! \brief Checks whether environment has store for some data type */
  template<typename T>
  bool has_store() const
  {
    constexpr auto key = store_info<T>::key;

    return stores.find( key ) != stores.end();
  }

public:
  inline std::ostream& out() const { return std::cout; }
  inline std::ostream& err() const { return std::cerr; }

private:
  template<class... S>
  friend class cli;

  friend class command;

  std::unordered_map<std::string, std::shared_ptr<void>> stores;

public:
  std::unordered_map<std::string, std::shared_ptr<command>> commands;
  std::unordered_map<std::string, std::vector<std::string>> categories;
  std::unordered_map<std::string, std::string> aliases;

public:
  bool log{false};
  alice::logger logger;
  bool quit{false};
};

class command
{
public:
  using rule_t = std::pair<std::function<bool()>, std::string>;
  using rules_t = std::vector<rule_t>;

  using log_map_t = logger::log_map_t;
  using log_opt_t = logger::log_opt_t;

  command( const environment::ptr& env, const std::string& caption )
      : env( env ),
        opts( caption ),
        scaption( caption )
  {
  }

  inline const auto& caption() const { return scaption; }

  virtual bool run( const std::vector<std::string>& args )
  {
    opts.reset();

    /* copy arguments (seems important to get the right grouping of arguments) */
    std::vector<char*> _args( args.size() );
    std::transform( args.begin(), args.end(), _args.begin(), []( const auto& s ) { return const_cast<char*>( s.c_str() ); } );

    try
    {
      opts.parse( _args.size(), &_args[0] );
    }
    catch ( const CLI::CallForHelp& e )
    {
      env->out() << opts.help();
      return false;
    }
    catch ( const CLI::ParseError& e )
    {
      env->err() << "[e] " << e.what() << std::endl;
      return false;
    }

    for ( const auto& p : validity_rules() )
    {
      if ( !p.first() )
      {
        env->err() << "[e] " << p.second << std::endl;
        return false;
      }
    }

    execute();
    return true;
  }

  inline bool is_set( const std::string& option ) const
  {
    assert( !option.empty() );

    if ( option.front() == '-' )
    {
      return opts.count( option );
    }
    else if ( option.size() == '1' )
    {
      return opts.count( "-" + option );
    }
    else
    {
      return opts.count( "--" + option );
    }
  }

protected:
  virtual rules_t validity_rules() const { return {}; }
  virtual void execute() = 0;

public:
  virtual log_opt_t log() const { return log_opt_t(); }

protected:
  environment::ptr env;

public:
  CLI::App opts;

private:
  std::string scaption;
};

template<typename S>
int add_option_helper( CLI::App& opts )
{
  constexpr auto option = store_info<S>::option;
  constexpr auto mnemonic = store_info<S>::mnemonic;
  constexpr auto name_plural = store_info<S>::name_plural;

  if ( strlen( mnemonic ) == 1u )
  {
    opts.add_flag( fmt::format( "-{},--{}", mnemonic, option ), name_plural );
  }
  else
  {
    opts.add_flag( fmt::format( "--{}", option ), name_plural );
  }
  return 0;
}

template<typename T>
bool any_true_helper( std::initializer_list<T> list )
{
  for ( auto i : list )
  {
    if ( i ) { return true; }
  }

  return false;
}

template<typename T>
bool exactly_one_true_helper( std::initializer_list<T> list )
{
  auto current = false;

  for ( auto i : list )
  {
    if ( i )
    {
      if ( current )
      {
        return false;
      }
      current = true;
    }
  }

  return current;
}
}
