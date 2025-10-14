#include <core_init.h>
#include <os_metrics.h>

PRAGMA_WARNING_SUPPRESS_ALL

extern "C" void MOVAllBytesASM(u64 Count, u8 *Data);
extern "C" void NOPAllBytesASM(u64 Count);
extern "C" void CMPAllBytesASM(u64 Count);
extern "C" void DECAllBytesASM(u64 Count);

core::Profiler profiler;

enum ProfilePoints {
    PP_RESERVED,

    PP_NOPAllBytesASM,
    PP_CMPAllBytesASM,
    PP_DECAllBytesASM,
};

i32 main() {
    coreInit();

    constexpr u64 N_COUNT = 1000000;

    profiler.beginProfile();

    {
        THROUGHPUT_BLOCK(profiler, PP_NOPAllBytesASM, "NOP all bytes ASM", N_COUNT);
        NOPAllBytesASM(N_COUNT);
    }
    {
        THROUGHPUT_BLOCK(profiler, PP_CMPAllBytesASM, "CMP all bytes ASM", N_COUNT);
        CMPAllBytesASM(N_COUNT);
    }
    {
        THROUGHPUT_BLOCK(profiler, PP_DECAllBytesASM, "DEC all bytes ASM", N_COUNT);
        DECAllBytesASM(N_COUNT);
    }

    auto profileRes = profiler.endProfile();
    logInfo("Profile 1");
    profileRes.logResult(core::LogLevel::L_INFO);

    return 0;
}
