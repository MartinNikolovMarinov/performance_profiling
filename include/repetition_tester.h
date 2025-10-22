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

    struct TestWave {
        TestFn fn;
        const char* label;

        u64 freq;
        u64 processedBytes;

        u32 runs;
        u32 unchanged;
        u32 repetitionMaxUnchanged;

        u64 totalTsc;
        u64 min;
        u64 max;
        u64 avg;

        u64 pfTotal;
        u64 pfMin;
        u64 pfMax;
        u64 pfAvg;
    };

    struct SingleTestStats {
        u64 pfBefore;
        u64 startTsc;
        u64 pfDiff;
        u64 elapsedTsc;
    };

    BeforeFn beforeEach = nullptr;
    core::ArrList<TestFnEntry> entries;

    // API to use the tester automatically:
    void setBeforeEach(BeforeFn fn);
    void registerFn(TestFn fn, const char* label, BeforeFn beforeFn = nullptr);
    void run(u32 repetitionMaxUnchanged);

    // Manual state management API:
    static TestWave startTestWave(u64 freq, u32 repetitionMaxUnchanged, const char* label);
    static void setProcessedBytes(TestWave& wave, u64 processedBytes);
    static void endTestWave(TestWave& wave);
    static SingleTestStats startSingleTest(TestWave& wave, bool& shouldStop);
    static void endSingleTest(TestWave& wave, SingleTestStats& s);
};

