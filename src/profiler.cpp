#include <profiler.h>

namespace {

constexpr addr_size MAX_TIMEPOINTS_COUNT = 4096;

struct Profiler {
    core::ArrStatic<ProfileTimePoint, MAX_TIMEPOINTS_COUNT> timepoints;
    u64 start;
    u64 end;
};

Profiler g_profiler;
u32 g_currTimepointIdx;

} // namespace

ProfileTimePoint& getProfileTimePoint(addr_size idx) {
    Assert(idx <= MAX_TIMEPOINTS_COUNT, "idx out of range");
    return g_profiler.timepoints[idx];
}

void beginProfile() {
    g_profiler.start = core::getPerfCounter();
    g_profiler.timepoints.replaceWith(ProfileTimePoint{}, MAX_TIMEPOINTS_COUNT);
    g_currTimepointIdx = 0;
}

ProfileResult endProfile() {
    g_profiler.end = core::getPerfCounter();

    ProfileResult result;
    u64 freq = core::getCPUFrequencyHz();
    Assert(freq != 0, "Estimated CPU frequency must not be 0");
    u64 totalElapsedTsc = g_profiler.end - g_profiler.start;

    result.cpuFrequencyHz = freq;
    result.totalElapsedTsc = totalElapsedTsc;
    result.totalElapsedNs = u64(core::CORE_SECOND * (f64(totalElapsedTsc) / f64(freq)));
    result.timepoints = core::Memory<ProfileTimePoint> { g_profiler.timepoints.data(), g_profiler.timepoints.len() };

    g_currTimepointIdx = 0;

    return result;
}

namespace {

inline void logElapsed(addr_size i, u64 totalElapsedTsc, u64 freq, const ProfileTimePoint& a) {
    char buffer[255];
    auto& label = a.label;
    u64 hits = a.hitCount;

    core::logDirectStd("%llu. %s\n", i, label);
    core::logDirectStd("    - Hits       : %llu\n", hits);

    // Elapsed only in current block
    {
        u64 tsc = a.elapsedTsc - a.elapsedChildrenTsc;
        u64 elapsedNS = u64(core::CORE_SECOND * (f64(tsc) / f64(freq)));
        core::testing::elapsedTimeToStr(buffer, elapsedNS);
        f64 percent = 100.0 * (f64(tsc) / f64(totalElapsedTsc));
        core::logDirectStd("    - Self Time  : %s (%llu, %.2f%%)\n", buffer, tsc, percent);
    }

    if (a.elapsedTsc != a.elapsedChildrenTsc) {
        u64 tsc = a.elapsedAtRootTsc;
        u64 elapsedNS = u64(core::CORE_SECOND * (f64(tsc) / f64(freq)));
        core::testing::elapsedTimeToStr(buffer, elapsedNS);
        f64 percent = 100.0 * (f64(tsc) / f64(totalElapsedTsc));
        core::logDirectStd("    - Total Time : %s (%llu, %.2f%%)\n", buffer, tsc, percent);
    }
};

} // namespace

void logProfileResult(const ProfileResult& result, core::LogLevel logLevel) {
    if (logLevel < core::getLogLevel()) {
        return;
    }

    u64 freq = result.cpuFrequencyHz;
    u64 totalElapsedNs = result.totalElapsedNs;
    u64 totalElapsedTsc = result.totalElapsedTsc;

    char totalElapsedStr[core::testing::ELAPSED_TIME_TO_STR_BUFFER_SIZE];
    core::testing::elapsedTimeToStr(totalElapsedStr, totalElapsedNs);

    core::logDirectStd("--- CPU Profile Summary ---\n");
    core::logDirectStd("CPU Frequency : %llu Hz (%.4f GHz)\n", freq, f64(freq) / 1000000000.0);
    core::logDirectStd("Total         : %s, %llu\n", totalElapsedStr, totalElapsedTsc);
     core::logDirectStd("\n");

    for (addr_size i = 0; i < result.timepoints.len(); i++) {
        auto& a = result.timepoints[i];
        if (a.isUsed()) {
            logElapsed(i, totalElapsedTsc, freq, a);
        }
    }
}

void __setGlobalProfileBlock(u32 idx) {
    g_currTimepointIdx = idx;
}

u32 __getGlobalProfileBlock() {
    return g_currTimepointIdx;
}
