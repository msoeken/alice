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
  \file cli.hpp
  \brief The main entry method for the CLI

  \author Mathias Soeken
*/

#pragma once

#include <chrono>
#include <fstream>
#include <memory>
#include <regex>
#include <string>

#include <CLI/CLI.hpp>
#include <fmt/format.h>

#include "command.hpp"
#include "logging.hpp"
#include "readline.hpp"

#include "commands/alias.hpp"
#include "commands/convert.hpp"
#include "commands/current.hpp"
#include "commands/help.hpp"
#include "commands/print.hpp"
#include "commands/ps.hpp"
#include "commands/quit.hpp"
#include "commands/read_io.hpp"
#include "commands/store.hpp"
#include "commands/write_io.hpp"

namespace alice
{

template<class... S>
class cli
{
public:
  cli( const std::string& prefix )
      : env( std::make_shared<environment>() ),
        prefix( prefix )
  {
    /* for each type in S ... */
    []( ... ) {}( ( env->add_store<S>(), 0 )... );

    set_category( "General" );
    insert_command( "alias", std::make_shared<alias_command>( env ) );
    insert_command( "convert", std::make_shared<convert_command<S...>>( env ) );
    insert_command( "current", std::make_shared<current_command<S...>>( env ) );
    insert_command( "help", std::make_shared<help_command>( env ) );
    insert_command( "print", std::make_shared<print_command<S...>>( env ) );
    insert_command( "ps", std::make_shared<ps_command<S...>>( env ) );
    insert_command( "quit", std::make_shared<quit_command>( env ) );
    insert_command( "store", std::make_shared<store_command<S...>>( env ) );

    opts.add_option( "-c,--command", command, "process semicolon-separated list of commands" );
    opts.add_option( "-f,--flag", file, "process file with new-line separated list of commands" );
    opts.add_flag( "-e,--echo", "echo the command if read from command line or file" );
    opts.add_flag( "-n,--counter", "show a counter in the prefix" );
    opts.add_flag( "-i,--interactive", "continue in interactive mode after processing commands (in command or file mode)" );
    opts.add_option( "-l,--log", logname, "logs the execution and stores many statistical information" );
  }

  void set_category( const std::string& _category )
  {
    category = _category;
  }

  void insert_command( const std::string& name, const std::shared_ptr<command>& cmd )
  {
    env->categories[category].push_back( name );
    env->commands[name] = cmd;
  }

  template<typename Tag>
  void insert_read_command( const std::string& name, const std::string& label )
  {
    insert_command( name, std::make_shared<read_io_command<Tag, S...>>( env, label ) );
  }

  template<typename Tag>
  void insert_write_command( const std::string& name, const std::string& label )
  {
    insert_command( name, std::make_shared<write_io_command<Tag, S...>>( env, label ) );
  }

  int run( int argc, char** argv )
  {
    try
    {
      opts.parse( argc, argv );
    }
    catch ( const CLI::CallForHelp& e )
    {
      env->out() << opts.help();
      return 1;
    }
    catch ( const CLI::ParseError& e )
    {
      env->out() << "[e] " << e.what() << std::endl;
      return 2;
    }

    // read_aliases();

    if ( opts.count( "-l" ) )
    {
      env->log = true;
      env->logger.start( logname );
    }

    if ( opts.count( "-c" ) )
    {
      auto split = detail::split_with_quotes<';'>( command );

      std::string batch_string;
      std::string abc_opts;
      for ( auto& line : split )
      {
        detail::trim( line );

        if ( opts.count( "-e" ) )
        {
          env->out() << get_prefix() << line << std::endl;
        }
        if ( !execute_line( preprocess_alias( line ) ) )
        {
          return 1;
        }

        if ( env->quit )
        {
          break;
        }
      }
    }
    else if ( opts.count( "-f" ) )
    {
      process_file( file, opts.count( "-e" ) );

      if ( !opts.count( "-i" ) )
      {
        env->quit = true;
      }
    }

    if ( ( !opts.count( "-c" ) && !opts.count( "-f" ) ) || ( !env->quit && opts.count( "-i" ) ) )
    {
      readline_wrapper rl( env );

      std::string line;
      while ( !env->quit && rl.read_command_line( get_prefix(), line ) )
      {
        execute_line( preprocess_alias( line ) );
        rl.add_to_history( line );
      }
    }

    if ( env->log )
    {
      env->logger.stop();
    }

    return 0;
  }

private:
  bool execute_line( const std::string& line )
  {
    /* at this point we can assume that line is trimmed */

    /* ignore comments and empty lines */
    if ( line.empty() || line[0] == '#' )
    {
      return false;
    }

    /* split commands if line contains a semi-colon */
    const auto lines = detail::split_with_quotes<';'>( line );

    /* if more than one command is detected recurse on each part */
    if ( lines.size() > 1u )
    {
      auto result = true;

      for ( const auto& cline : lines )
      {
        result = result && execute_line( preprocess_alias( cline ) );
      }

      return result;
    }

    /* escape to shell */
    if ( line[0] == '!' )
    {
      const auto now = std::chrono::system_clock::now();
      const auto result = detail::execute_program( line.substr( 1u ) );

      env->out() << result.second;

      if ( !result.second.empty() && result.second.back() != '\n' )
      {
        env->out() << '%' << std::endl;
      }

      if ( env->log )
      {
        auto log = std::make_shared<logger::log_map_t>();
        ( *log )["status"] = result.first;
        ( *log )["output"] = result.second;
        env->logger.log( log, line, now );
      }

      return true;
    }

    /* read commands from file */
    if ( line[0] == '<' )
    {
      auto filename = line.substr( 1u );
      detail::trim( filename );
      process_file( filename, opts.count( "-e" ) );
      return true;
    }

    auto vline = detail::split_with_quotes<' '>( line );

    const auto it = env->commands.find( vline.front() );
    if ( it != env->commands.end() )
    {
      const auto now = std::chrono::system_clock::now();
      const auto result = it->second->run( vline );

      if ( result && env->log )
      {
        env->logger.log( it->second->log(), line, now );
      }

      return result;
    }
    else
    {
      env->err() << "[e] unknown command: " << vline.front() << std::endl;
      return false;
    }

    return true;
  }

  bool process_file( const std::string& filename, bool echo )
  {
    std::ifstream in( filename.c_str(), std::ifstream::in );

    if ( !in.good() )
    {
      env->out() << "[e] file " << filename << " not found" << std::endl;
      return true;
    }

    std::string line;

    while ( getline( in, line ) )
    {
      detail::trim( line );

      if ( echo )
      {
        env->out() << get_prefix() << line << std::endl;
      }

      execute_line( preprocess_alias( line ) );

      if ( env->quit )
      {
        /* quit */
        return true;
      }
    }

    /* do not quit */
    return false;
  }

  std::string get_prefix()
  {
    if ( opts.count( "-n" ) )
    {
      return prefix + fmt::format( " {}> ", counter++ );
    }
    else
    {
      return prefix + "> ";
    }
  }

  std::string preprocess_alias( const std::string& line )
  {
    std::smatch m;

    for ( const auto& p : env->aliases )
    {
      if ( std::regex_match( line, m, std::regex( p.first ) ) )
      {
        std::vector<std::string> matches( m.size() - 1u );

        for ( auto i = 0u; i < matches.size(); ++i )
        {
          matches[i] = std::string( m[i + 1] );
        }

        const auto str = detail::trim_copy( detail::format_with_vector( p.second, matches ) );
        return preprocess_alias( str );
      }
    }

    return line;
  }

public:
  environment::ptr env;

private:
  std::string prefix;
  CLI::App opts;
  std::string category;

  std::string command, file, logname;

  unsigned counter{1u};
};
}
