cmake_minimum_required(VERSION 3.2)

project(performance_profiling)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(src_files
    src/init_core.cpp
    src/haversine.cpp
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/core)

# Create the generator executable:

add_executable(haversine_generator haversine_generator.cpp ${src_files})

target_include_directories(haversine_generator
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(haversine_generator PRIVATE -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-unused-function)

target_link_libraries(haversine_generator PRIVATE core)

# Create the parser executable:

add_executable(haversine_parser haversine_parser.cpp ${src_files})

target_include_directories(haversine_parser
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(haversine_parser PRIVATE -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-unused-function)

target_link_libraries(haversine_parser PRIVATE core)
