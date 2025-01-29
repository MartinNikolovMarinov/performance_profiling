cmake_minimum_required(VERSION 3.2)
project(STL_VIEWER VERSION 0.1.0 LANGUAGES CXX)

set(haversine_generator_target haversine_generator)

# Standard Requirements:

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generates compile_commands.json

# Add directories from lib cmake projects

set(CORE_ASSERT_ENABLED ON CACHE BOOL "Enable Asserts in corelib" FORCE)
add_subdirectory(lib/core)

# Include cmake modules from:

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_local")

include(DetectOS)
include(CompilerOptions)
include(Logger)
include(PerfAppDefaultFlags)

init_logger("[PERF_APP]")

# ---------------------------------------- Begin Options ---------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Release")
    set(PERF_APP_DEBUG OFF)
else()
    set(PERF_APP_DEBUG ON)
endif()

option(USE_ANSI_LOGGING "Use ANSI escape codes in logging." OFF)

option(CORE_LIBRARY_SHARED "Build core as a shared library." OFF)
option(CORE_ASSERT_ENABLED "Enable asserts." OFF)

# Print Selected Options:

log_info("---------------------------------------------")
log_info("Version:           ${PROJECT_VERSION}")
log_info("Platform:          ${OS}")
log_info("Arch:              ${CMAKE_SYSTEM_PROCESSOR}")
log_info("C++ Version:       ${CMAKE_CXX_STANDARD}")
log_info("Compiler:          ${CMAKE_CXX_COMPILER_ID}")
log_info("Debug:             ${PERF_APP_DEBUG}")
log_info("Use ANSI loagging: ${USE_ANSI_LOGGING}")
log_info("---------------------------------------------")

# ---------------------------------------- End Options -----------------------------------------------------------------

# ---------------------------------------- Begin Declare Source Files --------------------------------------------------

set(haversine_generator_src
    haversine_generator.cpp
    src/core_init.cpp
    src/haversine_distance.cpp
)

# ---------------------------------------- End Declare Source Files ----------------------------------------------------

# ---------------------------------------- Begin Create Executable -----------------------------------------------------

add_executable(${haversine_generator_target} ${haversine_generator_src})
target_link_libraries(${haversine_generator_target} PUBLIC
    core # link with corelib
)
target_include_directories(${haversine_generator_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(${haversine_generator_target} PUBLIC
    "PERF_APP_DEBUG=$<BOOL:${PERF_APP_DEBUG}>"
    "USE_ANSI_LOGGING=$<BOOL:${USE_ANSI_LOGGING}>"
    # PERF_APP_DATA_DIR="${CMAKE_BINARY_DIR}/data"
)
perf_app_target_set_default_flags(${haversine_generator_target} ${PERF_APP_DEBUG} false)

# ---------------------------------------- END Create Executable -------------------------------------------------------
