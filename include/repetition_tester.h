#pragma once

#include <core_init.h>

struct RepetitionTester {
    using TestFn = void (*)(u64& processedBytes);

    struct TestFnEntry {
        TestFn fn;
        const char* label;
        u64 processedBytes;
    };

    core::ArrList<TestFnEntry> entries;

    void registerFn(TestFn&& fn, const char* label);
    void run(u32 repetitionMaxUnchanged);
};

