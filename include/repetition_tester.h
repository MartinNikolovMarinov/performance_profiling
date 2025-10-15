#pragma once

#include <core_init.h>

struct RepetitionTester {
    using TestFn = void (*)(u64& processedBytes);
    using BeforeFn = void (*)();

    struct TestFnEntry {
        TestFn fn;
        BeforeFn beforeFn;
        const char* label;
        u64 processedBytes;
    };

    BeforeFn beforeEach = nullptr;
    core::ArrList<TestFnEntry> entries;

    void setBeforeEach(BeforeFn fn);
    void registerFn(TestFn fn, const char* label, BeforeFn beforeFn = nullptr);
    void run(u32 repetitionMaxUnchanged);
};

