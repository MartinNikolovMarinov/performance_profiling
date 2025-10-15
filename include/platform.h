#pragma once

#include <core_init.h>

u64 readOSPageFaultCount();

bool readOSRandomBytes(u64 count, void* dest);
template<typename T>
bool readOSRandomType(T& out) {
    return readOSRandomBytes(sizeof(T), &out);
}
