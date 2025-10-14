#include <core_init.h>
#include <os_metrics.h>

PRAGMA_WARNING_SUPPRESS_ALL

// FIXME: Next thing to write - the repetition tester to be able to analyze assembly functions easier with just adding
// them to a table.

struct Buffer {
    addr_size count;
    u8* data;
};

void WriteToAllBytes(Buffer dest) {
    for (u64 i = 0; i < dest.count; i++) {
        dest.data[i] = u8(i);
    }
}

extern "C" void MOVAllBytesASM(u64 Count, u8 *Data);
extern "C" void NOPAllBytesASM(u64 Count);
extern "C" void CMPAllBytesASM(u64 Count);
extern "C" void DECAllBytesASM(u64 Count);

core::Profiler profiler;

enum ProfilePoints {
    PP_RESERVED,

    PP_WriteToAllBytes,
    PP_MoveAllBytesASM,
    PP_NOPAllBytesASM,
    PP_CMPAllBytesASM,
    PP_DECAllBytesASM,
};

i32 main() {
    coreInit();

    auto verify = [](u8* arr, addr_size len) {
        for (u64 i = 0; i < len; i++) {
            Assert(arr[i] == u8(i));
        }
        core::memset(arr, u8(0), len);
    };

    constexpr addr_size N_COUNT = 1000000000;
    static u8 data[N_COUNT] = {};

    profiler.beginProfile();

    {
        THROUGHPUT_BLOCK(profiler, PP_WriteToAllBytes, "Write To All Bytes", N_COUNT);
        WriteToAllBytes({ N_COUNT, data });
    }

    verify(data, N_COUNT);

    {
        THROUGHPUT_BLOCK(profiler, PP_MoveAllBytesASM, "Move All Bytes ASM", N_COUNT);
        WriteToAllBytes({ N_COUNT, data });
    }

    verify(data, N_COUNT);

    {
        THROUGHPUT_BLOCK(profiler, PP_NOPAllBytesASM, "NOP All Bytes ASM", N_COUNT);
        NOPAllBytesASM(N_COUNT);
    }
    {
        THROUGHPUT_BLOCK(profiler, PP_CMPAllBytesASM, "CMP all bytes ASM", N_COUNT);
        CMPAllBytesASM(N_COUNT);
    }
    {
        THROUGHPUT_BLOCK(profiler, PP_DECAllBytesASM, "DEC All Bytes ASM", N_COUNT);
        DECAllBytesASM(N_COUNT);
    }

    auto profileRes = profiler.endProfile();
    logInfo("Profile 1");
    profileRes.logResult(core::LogLevel::L_INFO);

    return 0;
}
