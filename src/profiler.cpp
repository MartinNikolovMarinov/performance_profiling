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

void logElapsed(u64 totalElapsedTsc, u64 freq, const ProfileTimePoint& a) {
    // Elapsed in the entire time block:
    u64 entireBlockElapsedTsc = a.elapsedTsc;
    u64 entireBlockElapsedNS = u64(core::CORE_SECOND * (f64(entireBlockElapsedTsc) / f64(freq)));
    char entireBlockElapsedStr[255] = {};
    core::testing::elapsedTimeToStr(entireBlockElapsedStr, entireBlockElapsedNS);
    f64 entireBlockPercent = 100.0 * (f64(entireBlockElapsedTsc) / f64(totalElapsedTsc));

    // Elapsed only in current block
    u64 currBlockElapsedTsc = a.elapsedTsc - a.elapsedChildrenTsc;
    u64 currBlockElapsedNS = u64(core::CORE_SECOND * (f64(currBlockElapsedTsc) / f64(freq)));
    char currBlockElapsedStr[255] = {};
    core::testing::elapsedTimeToStr(currBlockElapsedStr, currBlockElapsedNS);
    f64 currBlockPercent = 100.0 * (f64(currBlockElapsedTsc) / f64(totalElapsedTsc));

    core::logDirectStd("    \"%s\": hits=%llu, tsc=%llu (%s, %.2f%%, %.2f%% w/children)\n",
                       a.label, a.hitCount,
                       currBlockElapsedTsc,
                       currBlockElapsedStr, currBlockPercent, entireBlockPercent);
};

} // namespace

void logProfileResult(const ProfileResult& result, core::LogLevel logLevel, u8 logTag) {
    if (logLevel < core::getLogLevel()) {
        return;
    }

    u64 freq = result.cpuFrequencyHz;
    u64 totalElapsedNs = result.totalElapsedNs;
    u64 totalElapsedTsc = result.totalElapsedTsc;

    char totalElapsedStr[core::testing::ELAPSED_TIME_TO_STR_BUFFER_SIZE];
    core::testing::elapsedTimeToStr(totalElapsedStr, totalElapsedNs);

    core::__log(logTag, logLevel, core::LogSpecialMode::SECTION_TITLE, "logProfileResult", "BEGIN CPU PROFILE");
    core::logDirectStd("  Estimated CPU Frequency: %lluHZ (%.4fGHZ)\n", freq, f64(freq) / 1000000000.0);
    core::logDirectStd("  Total: time=%s, tsc=%llu\n", totalElapsedStr, totalElapsedTsc);

    for (addr_size i = 0; i < result.timepoints.len(); i++) {
        auto& a = result.timepoints[i];
        if (a.isUsed()) {
            logElapsed(totalElapsedTsc, freq, a);
        }
    }
}

void __setGlobalProfileBlock(u32 idx) {
    g_currTimepointIdx = idx;
}

u32 __getGlobalProfileBlock() {
    return g_currTimepointIdx;
}
