add_library(alice INTERFACE)
target_include_directories(alice INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(alice INTERFACE fmt::fmt-header-only)
target_link_libraries(alice INTERFACE CLI11)
target_link_libraries(alice INTERFACE mpark_variant)
target_link_libraries(alice INTERFACE hana)

if(ALICE_READLINE_LIBRARY STREQUAL "READLINE")
  include(CheckCXXSourceRuns)
  set(CMAKE_REQUIRED_LIBRARIES readline)
  check_cxx_source_runs("
#include <stdio.h>
#include <readline/readline.h>
void foo() { readline(\"foo\"); }
int main() { return 0; }
" READLINE_WORKS)
  unset(CMAKE_REQUIRED_LIBRARIES)

  if( READLINE_WORKS )
    target_compile_definitions(alice INTERFACE "READLINE_USE_READLINE")
    target_link_libraries(alice INTERFACE readline)
  else()
    message(WARNING "Cannot find readline library, fall back to native interface.")
  endif()
endif()
