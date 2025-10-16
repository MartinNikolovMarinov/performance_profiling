#pragma once

#include <core.h>
#include <core_extensions/hash_functions.h>

using namespace coretypes;

void coreInit();

template<typename T>
T* defaultAlloc(addr_size count) {
    if constexpr (std::is_same_v<std::remove_cv_t<T>, void>) {
        return reinterpret_cast<T*>(core::getAllocator(core::DEFAULT_ALLOCATOR_ID).alloc(count, sizeof(char)));
    }
    else {
        return reinterpret_cast<T*>(core::getAllocator(core::DEFAULT_ALLOCATOR_ID).alloc(count, sizeof(T)));
    }
}

template<typename T>
T* defaultZeroAlloc(addr_size count) {
    if constexpr (std::is_same_v<std::remove_cv_t<T>, void>) {
        return reinterpret_cast<T*>(core::getAllocator(core::DEFAULT_ALLOCATOR_ID).zeroAlloc(count, sizeof(char)));
    }
    else {
        return reinterpret_cast<T*>(core::getAllocator(core::DEFAULT_ALLOCATOR_ID).zeroAlloc(count, sizeof(T)));
    }
}

template<typename T>
void defaultFree(T* addr, addr_size count) {
    if constexpr (std::is_same_v<std::remove_cv_t<T>, void>) {
        core::getAllocator(core::DEFAULT_ALLOCATOR_ID).free(reinterpret_cast<char*>(addr), count, sizeof(char));
    }
    else {
        core::getAllocator(core::DEFAULT_ALLOCATOR_ID).free(addr, count, sizeof(T));
    }
}

void logDirectStdF64(f64 f);
