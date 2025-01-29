#pragma once

#include <core.h>
#include <core_extensions/hash_functions.h>

using namespace coretypes;

void coreInit();

template<typename T>
T* defaultAlloc(addr_size count) {
    return reinterpret_cast<T*>(core::getAllocator(core::DEFAULT_ALLOCATOR_ID).alloc(count, sizeof(T)));
}

template<typename T>
void defaultFree(T* addr, addr_size count) {
    core::getAllocator(core::DEFAULT_ALLOCATOR_ID).free(addr, count, sizeof(T));
}

#define logF64(format, v, level) \
{ \
    constexpr addr_size buffLen = 32; \
    char buff[buffLen] = {}; \
    u32 n = core::Unpack(core::floatToCstr(v, buff, buffLen)); \
    __log(0, level, core::LogSpecialMode::NONE, __func__, format, n, buff); \
}

