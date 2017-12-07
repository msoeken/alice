include_directories(Catch2/single_include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -Wextra")

file(GLOB FILENAMES *.cpp)

add_executable(run_tests ${FILENAMES})
target_link_libraries(run_tests alice)
target_compile_definitions(run_tests PRIVATE ALICE_NOAPI)
