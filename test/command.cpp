#include <catch.hpp>

#include <string>

#include <alice/alice.hpp>

using namespace alice;

namespace alice
{

template<>
struct store_info<std::string>
{
  static constexpr const char* key = "strings";
  static constexpr const char* name = "string";
};

}

TEST_CASE( "Basic environment operations", "[environment]" )
{
  environment env;
  env.add_store<std::string>();
  CHECK( env.has_store<std::string>() );
  CHECK( env.store<std::string>().empty() );
}

