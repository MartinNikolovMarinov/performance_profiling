#pragma once

// IMPORTANT:
// This file initializes default settings for core and std core. Anywhere the application wants to use somthing from
// core or std core, it must include this file instead of directly including them.
// This is because defining default macros can be done only here.

#include <std/allocators/alloc_std_stats.h>

using namespace coretypes;

// Define the default allocator to be used by core:

static core::std_stats_allocator g_stdAlloc;
struct std_allocator_static {
    static void* alloc(addr_size size) noexcept;

    static void* calloc(addr_size count, addr_size size) noexcept;

    static void free(void* ptr) noexcept;

    static addr_size used_mem() noexcept;

    static const char* allocator_name() noexcept;
};

#undef CORE_DEFAULT_ALLOCATOR
#define CORE_DEFAULT_ALLOCATOR() std_allocator_static

#include <core.h>
#include <std/core.h>

#include <fmt/core.h>
#include <fmt/color.h>

void initCore();

using sb = core::str_builder<>;
