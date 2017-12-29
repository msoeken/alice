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

#include <experimental/any>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <json.hpp>

#include "detail/utils.hpp"
#include "logging.hpp"
#include "store.hpp"
#include "store_api.hpp"

namespace alice
{

class command;

template<class... S>
class cli;

/*! \brief Shell environment

  The environment gives access to shell related properties, e.g., the commands
  and stores.
*/
class environment
{
public:
  /*! \brief Smart pointer alias for environment */
  using ptr = std::shared_ptr<environment>;

  /*! \brief Retrieves store from environment

    The store can be accessed using its type.
  */
  template<typename T>
  store_container<T>& store() const
  {
    constexpr auto key = store_info<T>::key;

    return *( reinterpret_cast<alice::store_container<T>*>( _stores.at( key ).get() ) );
  }

  /*! \brief Checks whether environment has store for some data type
  
    Stores are defined by their type.
  */
  template<typename T>
  bool has_store() const
  {
    constexpr auto key = store_info<T>::key;

    return _stores.find( key ) != _stores.end();
  }

  /*! \brief Retreives standard output stream

    This method returns a reference to the current standard output stream.  In
    stand-alone application mode, this is ``std::cout`` by default, but can be
    changed.  Users should aim for not printing to ``std::cout`` directly in a
    command, but use ``env->out()`` instead.
  */
  inline std::ostream& out() const { return *_out; }

  /*! \brief Retreives standard error stream

    This method returns a reference to the current standard error stream.  In
    stand-alone application mode, this is ``std::cerr`` by default, but can be
    changed.  Users should aim for not printing to ``std::cerr`` directly in a
    command, but use ``env->err()`` instead.
  */
  inline std::ostream& err() const { return *_err; }

  /*! \brief Changes output and error streams

    This method allows to change the output streams which are returned by
    ``out()`` and ``err()``.
  */
  inline void reroute( std::ostream& new_out, std::ostream& new_err )
  {
    _out = &new_out;
    _err = &new_err;
  }

  /*! \brief Returns map of commands
  
    The keys correspond to the command names in the shell.
  */
  inline const std::unordered_map<std::string, std::shared_ptr<command>>& commands() const
  {
    return _commands;
  }

  /*! \brief Returns map of categories
  
    Keys are catgory names pointing to a vector of command names that can be
    used to index into ``commands()``.
  */
  inline const std::unordered_map<std::string, std::vector<std::string>>& categories() const
  {
    return _categories;
  }

  /*! \brief Returns a map of aliases

    Keys are the alias regular expressions mapping to substitutions.
  */
  inline const std::unordered_map<std::string, std::string>& aliases() const
  {
    return _aliases;
  }

private:
  /*! \brief Adds store to environment */
  template<typename T>
  void add_store()
  {
    constexpr auto key = store_info<T>::key;
    constexpr auto name = store_info<T>::name;

    _stores.emplace( key, std::shared_ptr<void>( new alice::store_container<T>( name ) ) );
  }

private:
  /* cli is friend to add stores, categories, and commands */
  template<class... S>
  friend class cli;

  /* help command is friend to sort categories */
  friend class help_command;

  /* alias command is friend to insert aliases */
  friend class alias_command;

  /* quit command is friend to update quit flag */
  friend class quit_command;

private:
  std::unordered_map<std::string, std::shared_ptr<void>> _stores;
  std::unordered_map<std::string, std::shared_ptr<command>> _commands;
  std::unordered_map<std::string, std::vector<std::string>> _categories;
  std::unordered_map<std::string, std::string> _aliases;

  bool log{false};
  alice::logger logger;
  bool quit{false};

  std::ostream* _out = &std::cout;
  std::ostream* _err = &std::cerr;
};

class command
{
public:
  using rule_t = std::pair<std::function<bool()>, std::string>;
  using rules_t = std::vector<rule_t>;

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

  inline auto add_flag( const std::string& name, const std::string& description = std::string() )
  {
    return opts.add_flag( name, description );
  }

  template<typename T = std::string>
  inline auto add_option( const std::string& name, const std::string& description = std::string() )
  {
    const auto index = options.size();
    options.push_back( T() );
    auto opt = opts.add_option( name, options.back(), description );

    for ( const auto& name : detail::split( opt->get_name(), "," ) )
    {
      option_index[name] = index;
    }

    return opt;
  }

  template<typename T = std::string>
  inline T option_value( const std::string& name, const T& default_value = T() )
  {
    const auto it = option_index.find( name );
    if ( it == option_index.end() )
    {
      return default_value;
    }
    else
    {
      const std::experimental::any& a = options.at( it->second );
      return *std::experimental::any_cast<T>( &a );
    }
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

  template<typename T>
  inline store_container<T>& store() const
  {
    return env->store<T>();
  }

protected:
  virtual rules_t validity_rules() const { return {}; }
  virtual void execute() = 0;

public:
  virtual nlohmann::json log() const { return nullptr; }

protected:
  environment::ptr env;

public:
  CLI::App opts;

private:
  std::string scaption;
  std::vector<std::experimental::any> options;
  std::unordered_map<std::string, unsigned> option_index;

public:
  template<typename StoreType, typename Tag>
  friend bool can_read( command& cmd );

  template<typename StoreType, typename Tag>
  friend StoreType read( const std::string& filename, command& cmd );

  template<typename StoreType, typename Tag>
  friend bool can_write( command& cmd );
  template<typename StoreType, typename Tag>
  friend void write( const StoreType& element, const std::string& filename, command& cmd );
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
    if ( i )
    {
      return true;
    }
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
