#include <core_init.h>

struct Pair {
    f64 x0, y0, x1, y1;

    inline void __debugTraceLog() {
        if (core::loggerGetLevel() <= core::LogLevel::L_TRACE) {
            core::logDirectStd("Pair: {");
            core::logDirectStd("x0="); logDirectStdF64(x0);
            core::logDirectStd(", y0="); logDirectStdF64(y0);
            core::logDirectStd(", x1="); logDirectStdF64(x1);
            core::logDirectStd(", y1="); logDirectStdF64(y1);
            core::logDirectStd("}");
            core::logDirectStd("\n");
        }
    }
};
