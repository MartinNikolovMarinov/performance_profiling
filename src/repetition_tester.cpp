#include <repetition_tester.h>

void RepetitionTester::registerFn(TestFn&& fn, const char* label) {
    entries.push({std::move(fn), label, 0});
}

void RepetitionTester::run(u32 repetitionMaxUnchanged) {
    auto freq = core::getCPUFrequencyHz();

    for (addr_size i = 0; i < entries.len(); i++) {
        u32 runs = 0;
        u32 unchanged = 0;
        u64 min = core::limitMax<u64>();
        u64 max = core::limitMin<u64>();
        u64 totalTsc = 0;
        u64 avg = 0;

        // Run the test functions for some fixed iteration count and find the fastest execution time.
        while (unchanged < repetitionMaxUnchanged) {
            u64 startTsc = core::getPerfCounter();
            entries[i].fn(entries[i].processedBytes);
            u64 elapsedTsc = core::getPerfCounter() - startTsc;
            totalTsc += elapsedTsc;

            if (elapsedTsc < min) {
                min = elapsedTsc;
                unchanged = 0; // reset only when a new min is found.
            }
            else if (elapsedTsc > max) {
                max = elapsedTsc;
            }
            else {
                unchanged++;
            }

            runs++;
        }

        avg = totalTsc / runs;

        auto processedBytes = entries[i].processedBytes;
        char elapsedBuf[core::testing::ELAPSED_TIME_TO_STR_BUFFER_SIZE];
        char processedBytesBuf[core::testing::MEMORY_USED_TO_STR_BUFFER_SIZE];
        core::testing::memoryUsedToStr(processedBytesBuf, u64(f64(processedBytes) / (f64(min) / f64(freq))));

        core::logDirectStd("--- {} ---\n", entries[i].label);
        {
            core::testing::elapsedTimeToStr(elapsedBuf, min);
            core::logDirectStd("  Min: {} ({}) {}/s\n", elapsedBuf, min, processedBytesBuf);
        }
        {
            core::testing::elapsedTimeToStr(elapsedBuf, max);
            core::logDirectStd("  Max: {} ({}) {}/s\n", elapsedBuf, max, processedBytesBuf);
        }
        {
            core::testing::elapsedTimeToStr(elapsedBuf, avg);
            core::logDirectStd("  Avg: {} ({}) {}/s\n", elapsedBuf, avg, processedBytesBuf);
        }
        core::logDirectStd("\n");
    }
}
