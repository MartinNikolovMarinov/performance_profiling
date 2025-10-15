#include <repetition_tester.h>
#include <platform.h>

void RepetitionTester::setBeforeEach(BeforeFn fn) {
    beforeEach = fn;
}

void RepetitionTester::registerFn(TestFn fn, const char* label, BeforeFn beforeFn) {
    entries.push({fn, beforeFn, label, 0});
}

void RepetitionTester::run(u32 repetitionMaxUnchanged) {
    auto freq = core::getCPUFrequencyHz();

    for (addr_size i = 0; i < entries.len(); i++) {
        u32 runs = 0;
        u32 unchanged = 0;

        u64 totalTsc = 0;
        u64 min = core::limitMax<u64>();
        u64 max = core::limitMin<u64>();
        u64 avg = 0;

        u64 pfTotal = 0;
        u64 pfMin = core::limitMax<u64>();
        u64 pfMax = core::limitMin<u64>();
        u64 pfAvg = 0;

        while (unchanged < repetitionMaxUnchanged) {
            if (beforeEach) beforeEach();
            if (entries[i].beforeFn) entries[i].beforeFn();

            u64 pfBefore = readOSPageFaultCount();
            u64 startTsc = core::getPerfCounter();
            entries[i].fn(entries[i].processedBytes);
            u64 elapsedTsc = core::getPerfCounter() - startTsc;
            u64 pfDiff = readOSPageFaultCount() - pfBefore;

            if (elapsedTsc < min) {
                min = elapsedTsc;
                pfMin = pfDiff;
                unchanged = 0;
            } else if (elapsedTsc > max) {
                max = elapsedTsc;
                pfMax = pfDiff;
            } else {
                unchanged++;
            }

            totalTsc += elapsedTsc;
            pfTotal += pfDiff;
            runs++;
        }

        avg = totalTsc / runs;
        pfAvg = pfTotal / runs;

        auto processedBytes = entries[i].processedBytes;
        char elapsedBuf[core::testing::ELAPSED_TIME_TO_STR_BUFFER_SIZE];
        char processedBytesBuf[core::testing::MEMORY_USED_TO_STR_BUFFER_SIZE];
        core::testing::memoryUsedToStr(processedBytesBuf, u64(f64(processedBytes) / (f64(min) / f64(freq))));

        core::logDirectStd("--- {} ---\n", entries[i].label);
        {
            core::testing::elapsedTimeToStr(elapsedBuf, min);
            if (pfMin > 0)
                core::logDirectStd("  Min: {} ({}) {}/s | PF: {}\n", elapsedBuf, min, processedBytesBuf, pfMin);
            else
                core::logDirectStd("  Min: {} ({}) {}/s\n", elapsedBuf, min, processedBytesBuf);
        }
        {
            core::testing::elapsedTimeToStr(elapsedBuf, max);
            if (pfMax > 0)
                core::logDirectStd("  Max: {} ({}) {}/s | PF: {}\n", elapsedBuf, max, processedBytesBuf, pfMax);
            else
                core::logDirectStd("  Max: {} ({}) {}/s\n", elapsedBuf, max, processedBytesBuf);
        }
        {
            core::testing::elapsedTimeToStr(elapsedBuf, avg);
            if (pfAvg > 0)
                core::logDirectStd("  Avg: {} ({}) {}/s | PF: {}\n", elapsedBuf, avg, processedBytesBuf, pfAvg);
            else
                core::logDirectStd("  Avg: {} ({}) {}/s\n", elapsedBuf, avg, processedBytesBuf);
        }
        core::logDirectStd("\n");
    }
}

