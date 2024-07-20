cmake_minimum_required(VERSION 3.2)

project(performance VERSION 0.0.1)

set(v_HAVERSINE_GENERATOR_NAME "haversine_generator")
set(v_HAVERSINE_GENERATOR_NAME_UPPER "HAVERSINE_GENERATOR")
set(v_HAVERSINE_PARSER_NAME "haversine_parser")
set(v_HAVERSINE_PARSER_NAME_UPPER "HAVERSINE_PARSER")

# Standard Requirements:

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generates compile_commands.json

# Includes:

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(DetectOS)
include(CompilerOptions)

# Download and update git submodules:

# NOTE: That this function can NOT exist in a git submodule!
function(update_git_submodules)
    find_package(Git QUIET)
    if (GIT_FOUND)
        message(STATUS "Submodule update")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT
        )
        if (NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    else()
        message(FATAL_ERROR "Git is not found. It is required to update submodules.")
    endif()
endfunction()

update_git_submodules()

# ---------------------------------------- Begin Options ---------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Release")
    set(APP_DEBUG OFF)
    set(APP_BUILD_TESTS OFF)
else()
    set(APP_DEBUG ON)
    set(APP_BUILD_TESTS ON)
endif()

# Print Selected Options:

message(STATUS "[APP] Version:     ${PROJECT_VERSION}")
message(STATUS "[APP] Platform:    ${OS}")
message(STATUS "[APP] Arch:        ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message(STATUS "[APP] C++ Version: ${CMAKE_CXX_STANDARD}")
message(STATUS "[APP] Compiler:    ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "[APP] Debug:       ${APP_DEBUG}")

# ---------------------------------------- End Options -----------------------------------------------------------------

# ---------------------------------------- Begin Declare Source Files --------------------------------------------------

set(v_COMMON_SRC_FILES
    ./src/core_init.cpp
)

set(v_HAVERSINE_GENERATOR_SRC_FILES
    haversine_generator.cpp
)

set(v_HAVERSINE_PARSER_SRC_FILES
    haversine_parser.cpp
)

# ---------------------------------------- End Declare Source Files ----------------------------------------------------

# Add Libraries:

add_subdirectory(lib/core)

# Create Executables:

add_executable(${v_HAVERSINE_GENERATOR_NAME} ${v_COMMON_SRC_FILES} ${v_HAVERSINE_GENERATOR_SRC_FILES})
target_link_libraries(${v_HAVERSINE_GENERATOR_NAME} core)

add_executable(${v_HAVERSINE_PARSER_NAME} ${v_COMMON_SRC_FILES} ${v_HAVERSINE_PARSER_SRC_FILES})
target_link_libraries(${v_HAVERSINE_PARSER_NAME} core)

# Include Path:

target_include_directories(${v_HAVERSINE_GENERATOR_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(${v_HAVERSINE_PARSER_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Definitions:

target_compile_definitions(${v_HAVERSINE_GENERATOR_NAME} PRIVATE
    "APP_DEBUG=$<BOOL:${APP_DEBUG}>"
)

target_compile_definitions(${v_HAVERSINE_PARSER_NAME} PRIVATE
    "APP_DEBUG=$<BOOL:${APP_DEBUG}>"
)

# ---------------------------------------- Begin Compile Options -------------------------------------------------------

macro(target_set_default_flags target)

    # Flags specific to the projects:
    if(OS STREQUAL "linux")
        set(common_flags "-std=c++20" "-pthread")
        set(debug_flags "")
        set(release_flags "")
    elseif(OS STREQUAL "darwin")
        set(common_flags "-std=c++20" "-pthread")
        set(debug_flags "")
        set(release_flags "")
    elseif(OS STREQUAL "windows")
        set(common_flags "/std:c++20")
        set(debug_flags "")
        set(release_flags "")
    endif()

    generate_common_flags(
        common_flags "${common_flags}"
        debug_flags "${debug_flags}"
        release_flags "${release_flags}"
    )

    if(APP_DEBUG)
        target_compile_options(${target} PRIVATE ${common_flags} ${debug_flags})
    else()
        target_compile_options(${target} PRIVATE ${common_flags} ${release_flags})
    endif()

endmacro()

target_set_default_flags(${v_HAVERSINE_GENERATOR_NAME})
target_set_default_flags(${v_HAVERSINE_PARSER_NAME})

# ---------------------------------------- End Compile Options ---------------------------------------------------------

if(CORE_BUILD_TESTS)
    # TODO: Add tests here
endif()
