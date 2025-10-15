#include <core_init.h>
#include <platform.h>
#include <repetition_tester.h>

PRAGMA_WARNING_SUPPRESS_ALL

// ############################################### FUNCTIONS TO PROFILE ################################################

extern "C" void MOVAllBytesASM(u64 count, u8 *data);
extern "C" void NOPAllBytesASM(u64 count);
extern "C" void CMPAllBytesASM(u64 count);
extern "C" void DECAllBytesASM(u64 count);

extern "C" void NOP3x1AllBytes(u64 count);
extern "C" void NOP1x3AllBytes(u64 count);
extern "C" void NOP1x9AllBytes(u64 count);

extern "C" void ConditionalNOP(u64 count, u64 value);

struct Buffer {
    addr_size count;
    u8* data;
};

void WriteToAllBytes(Buffer dest) {
    for (u64 i = 0; i < dest.count; i++) {
        dest.data[i] = u8(i);
    }
}

// ##################################################### HELPERS #######################################################

constexpr addr_size N_COUNT = 100000000;
static u8 g_data[N_COUNT] = {};

void verifyAndClearData(u8* arr, addr_size len) {
    for (u64 i = 0; i < len; i++) {
        Assert(arr[i] == u8(i));
    }
    core::memset(arr, u8(0), len);
};

void WriteToAllBytes_TestFn(u64& processedBytes) {
    WriteToAllBytes({ N_COUNT, g_data });
    // verifyAndClearData(g_data, N_COUNT);
    processedBytes = N_COUNT;
}

void MOVAllBytesASM_TestFn(u64& processedBytes) {
    MOVAllBytesASM(N_COUNT, g_data);
    // verifyAndClearData(g_data, N_COUNT);
    processedBytes = N_COUNT;
}

void NOPAllBytesASM_TestFn(u64& processedBytes) {
    NOPAllBytesASM(N_COUNT);
    processedBytes = N_COUNT;
}

void CMPAllBytesASM_TestFn(u64& processedBytes) {
    CMPAllBytesASM(N_COUNT);
    processedBytes = N_COUNT;
}

void DECAllBytesASM_TestFn(u64& processedBytes) {
    DECAllBytesASM(N_COUNT);
    processedBytes = N_COUNT;
}

void NOP3x1AllBytes_TestFn(u64& processedBytes) {
    NOP3x1AllBytes(N_COUNT);
    processedBytes = N_COUNT;
}

void NOP1x3AllBytes_TestFn(u64& processedBytes) {
    NOP1x3AllBytes(N_COUNT);
    processedBytes = N_COUNT;
}

void NOP1x9AllBytes_TestFn(u64& processedBytes) {
    NOP1x9AllBytes(N_COUNT);
    processedBytes = N_COUNT;
}

// ###################################################### MAIN #########################################################

i32 main() {
    coreInit();

    RepetitionTester tester = {};

    // These exampes compare the simplest implementation of write to all bytes in C++ code and in ASM. After that it
    // tests what happens to the performance after some of the instructions are removed or replaced with noop.
    {
        tester.registerFn(WriteToAllBytes_TestFn, "WriteToAllBytes");
        tester.registerFn(MOVAllBytesASM_TestFn, "MOVAllBytesASM");
        tester.registerFn(NOPAllBytesASM_TestFn, "NOPAllBytesASM");
        tester.registerFn(CMPAllBytesASM_TestFn, "CMPAllBytesASM");
        tester.registerFn(DECAllBytesASM_TestFn, "DECAllBytesASM");
    }

    // These Testes demonstrate what happens when the CPU front-end architecture becomes the bottleneck. They
    // demonstrate how the CPU frontend (fetch/decode/dispatch) and backend (execution units, retirement) interact when
    // executing instructions that do no real work.
    {
        tester.registerFn(NOP3x1AllBytes_TestFn, "NOP3x1AllBytes");
        tester.registerFn(NOP1x3AllBytes_TestFn, "NOP1x3AllBytes");
        tester.registerFn(NOP1x9AllBytes_TestFn, "NOP1x9AllBytes");
    }

    logInfo("Repetition Testing:\n");
    tester.run(10);

    return 0;
}
