cmake_minimum_required(VERSION 3.2)

project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(src_files
    src/init_core.cpp
)

# Create the executable

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp ${src_files})

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-unused-function)

# Core library:
target_link_libraries(${PROJECT_NAME} PRIVATE core)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/core)

# Fmt library:
target_link_libraries(${PROJECT_NAME} PRIVATE fmt)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt)
target_include_directories(fmt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt/include)
