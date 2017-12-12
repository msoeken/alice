# No debugging and optimization for examples
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG")

function(add_example name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} alice)
endfunction()

add_example(demo)

add_alice_python_module(demo demo.cpp)

# tutorial 1
add_executable(tutorial1 tutorial1/tutorial1.cpp)
target_link_libraries(tutorial1 alice)

# tutorial 2
add_executable(tutorial2 tutorial2/tutorial2.cpp)
target_link_libraries(tutorial2 alice)

# tutorial 4
add_executable(tutorial4 tutorial4/tutorial4.cpp)
target_link_libraries(tutorial4 alice)
