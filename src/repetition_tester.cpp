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
        auto wave = startTestWave(freq, repetitionMaxUnchanged, entries[i].label);
        wave.fn = entries[i].fn;

        while (true) {
            if (beforeEach) beforeEach();
            if (entries[i].beforeFn) entries[i].beforeFn();

            bool shouldStop = false;
            auto t = startSingleTest(wave, shouldStop);
            wave.fn(wave.processedBytes); // The only part of the code that is beeing measured.
            endSingleTest(wave, t);
            if (shouldStop) break;
        }

        endTestWave(wave);
    }
}

RepetitionTester::TestWave RepetitionTester::startTestWave(u64 freq, u32 repetitionMaxUnchanged, const char* label) {
    TestWave ret;

    ret.freq = freq;
    ret.label = label;
    ret.processedBytes = 0;

    ret.runs = 0;
    ret.unchanged = 0;
    ret.repetitionMaxUnchanged = repetitionMaxUnchanged;

    ret.totalTsc = 0;
    ret.min = core::limitMax<u64>();
    ret.max = core::limitMin<u64>();
    ret.avg = 0;

    ret.pfTotal = 0;
    ret.pfMin = core::limitMax<u64>();
    ret.pfMax = core::limitMin<u64>();
    ret.pfAvg = 0;

    return  ret;
}

void RepetitionTester::setProcessedBytes(TestWave& wave, u64 processedBytes) {
    wave.processedBytes = processedBytes;
}

void RepetitionTester::endTestWave(TestWave& wave) {
    wave.avg = wave.totalTsc / wave.runs;
    wave.pfAvg = wave.pfTotal / wave.runs;

    auto processedBytes = wave.processedBytes;
    char elapsedBuf[core::testing::ELAPSED_TIME_TO_STR_BUFFER_SIZE];
    char processedBytesBuf[core::testing::MEMORY_USED_TO_STR_BUFFER_SIZE];
    core::testing::memoryUsedToStr(processedBytesBuf, u64(f64(processedBytes) / (f64(wave.min) / f64(wave.freq))));

    core::logDirectStd("--- {} ---\n", wave.label);
    {
        core::testing::elapsedTimeToStr(elapsedBuf, wave.min);
        if (wave.pfMin > 0)
            core::logDirectStd("  Min: {} ({}) {}/s | PF: {}\n", elapsedBuf, wave.min, processedBytesBuf, wave.pfMin);
        else
            core::logDirectStd("  Min: {} ({}) {}/s\n", elapsedBuf, wave.min, processedBytesBuf);
    }
    {
        core::testing::elapsedTimeToStr(elapsedBuf, wave.max);
        if (wave.pfMax > 0)
            core::logDirectStd("  Max: {} ({}) {}/s | PF: {}\n", elapsedBuf, wave.max, processedBytesBuf, wave.pfMax);
        else
            core::logDirectStd("  Max: {} ({}) {}/s\n", elapsedBuf, wave.max, processedBytesBuf);
    }
    {
        core::testing::elapsedTimeToStr(elapsedBuf, wave.avg);
        if (wave.pfAvg > 0)
            core::logDirectStd("  Avg: {} ({}) {}/s | PF: {}\n", elapsedBuf, wave.avg, processedBytesBuf, wave.pfAvg);
        else
            core::logDirectStd("  Avg: {} ({}) {}/s\n", elapsedBuf, wave.avg, processedBytesBuf);
    }
    core::logDirectStd("\n");
}

RepetitionTester::SingleTestStats RepetitionTester::startSingleTest(TestWave& wave, bool& shouldStop) {
    shouldStop = wave.unchanged >= wave.repetitionMaxUnchanged;

    SingleTestStats ret;
    ret.pfBefore = readOSPageFaultCount();
    ret.startTsc = core::getPerfCounter();
    return ret;
}

void RepetitionTester::endSingleTest(TestWave& wave, SingleTestStats& s) {
    u64 elapsedTsc = core::getPerfCounter() - s.startTsc;
    u64 pfDiff = readOSPageFaultCount() - s.pfBefore;

    if (elapsedTsc < wave.min) {
        wave.min = elapsedTsc;
        wave.pfMin = pfDiff;
        wave.unchanged = 0;
    } else if (elapsedTsc > wave.max) {
        wave.max = elapsedTsc;
        wave.pfMax = pfDiff;
    } else {
        wave.unchanged++;
    }

    wave.totalTsc += elapsedTsc;
    wave.pfTotal += pfDiff;
    wave.runs++;
}
