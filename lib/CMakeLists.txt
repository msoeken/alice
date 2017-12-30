#add_subdirectory(fmt EXCLUDE_FROM_ALL)
add_subdirectory(CLI11 EXCLUDE_FROM_ALL)
add_subdirectory(pybind11 EXCLUDE_FROM_ALL)

add_library(alice_json INTERFACE) # v3.0.0
target_include_directories(alice_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json)

add_library(alice_fmt INTERFACE)
target_include_directories(alice_fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
target_compile_definitions(alice_fmt INTERFACE FMT_HEADER_ONLY)
