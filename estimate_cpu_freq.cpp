#include <core_init.h>

PRAGMA_WARNING_SUPPRESS_ALL

i32 main() {
    coreInit();

    u64 estimatedCPUFrequency = core::getCPUFrequencyHz();

    core::logDirectStd("CPU Frequency : {} Hz ({:f.4} GHz)\n",
        estimatedCPUFrequency,
        f64(estimatedCPUFrequency) / 1000000000.0);

    return 0;
}
