cmake_minimum_required(VERSION 3.2)

project(performance_profiling)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(src_files
    src/init_core.cpp
)

# Create the executable

add_executable(haversine_generator haversine_generator.cpp ${src_files})

target_include_directories(haversine_generator
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(haversine_generator PRIVATE -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-unused-function)

# Core library:
target_link_libraries(haversine_generator PRIVATE core)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/core)

# Fmt library:
target_link_libraries(haversine_generator PRIVATE fmt)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt)
target_include_directories(fmt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt/include)
