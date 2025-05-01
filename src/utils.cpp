#include <utils.h>

void printTime(const char* label, f64 CPUTime, u64 _CPUTimerFreq) {
    core::logDirectStd("%s: %.0f", label, CPUTime);
    if (_CPUTimerFreq > 0) {
        f64 seconds = CPUTime / f64(_CPUTimerFreq);
        core::logDirectStd(" (%fms)", 1000.0 * seconds);
    }
}

void printTime(const char* label, u64 CPUTime, u64 _CPUTimerFreq) {
    printTime(label, f64(CPUTime), _CPUTimerFreq);
}

void printTimeWithBandwith(const char* label, f64 CPUTime, u64 _CPUTimerFreq, u64 byteCount) {
    core::logDirectStd("%s: %.0f", label, CPUTime);
    if (_CPUTimerFreq > 0) {
        f64 seconds = CPUTime / f64(_CPUTimerFreq);
        core::logDirectStd(" (%fms)", 1000.0 * seconds);

        if (byteCount > 0) {
            f64 bestBandwidth = f64(byteCount) / (f64(core::CORE_GIGABYTE) * seconds);
            core::logDirectStd(" %fgb/s", bestBandwidth);
        }
    }
}

void printTimeWithBandwith(const char* label, u64 CPUTime, u64 _CPUTimerFreq, u64 byteCount) {
    printTimeWithBandwith(label, f64(CPUTime), _CPUTimerFreq, byteCount);
}
