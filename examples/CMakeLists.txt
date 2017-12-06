# No debugging and optimization for examples
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG")

function(add_example name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} alice)
endfunction()

add_example(demo)

add_library(demo_python MODULE demo.cpp)
target_link_libraries(demo_python PRIVATE alice_python)
set_target_properties(demo_python PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}" OUTPUT_NAME "demo" SUFFIX "${PYTHON_MODULE_EXTENSION}")

# tutorial 1
add_executable(tutorial1 tutorial1/tutorial1.cpp)
target_link_libraries(tutorial1 alice)
