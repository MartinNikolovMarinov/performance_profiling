#include <core_init.h>
#include <platform.h>
#include <repetition_tester.h>

PRAGMA_WARNING_SUPPRESS_ALL

// ############################################### FUNCTIONS TO PROFILE ################################################

extern "C" void Read_x1(u64 count, u8* out);
extern "C" void Read_x2(u64 count, u8* out);
extern "C" void Read_x3(u64 count, u8* out);
extern "C" void Read_x4(u64 count, u8* out);
extern "C" void Read_1x2(u64 count, u8* out);

extern "C" void Write_x1(u64 count, u8* in);
extern "C" void Write_x2(u64 count, u8* in);
extern "C" void Write_x3(u64 count, u8* in);
extern "C" void Write_x4(u64 count, u8* in);
extern "C" void Write_1x2(u64 count, u8* in);

// ##################################################### HELPERS #######################################################

constexpr addr_size N_COUNT = 1000000000;
u8 g_data[sizeof(addr_size)] = {};

void cleanBuffer() {
    core::memset(g_data, u8(0),  CORE_C_ARRLEN(g_data));
}

void verifyWrite() {
    for (addr_size i = 0; i < CORE_C_ARRLEN(g_data); i++) {
        AssertFmt(g_data[i] == 0xFF, "verifyWrite failed at data[{}]={} != 0xFF", i, g_data[i]);
    }
}

// ###################################################### TEST FUNCTIONS ###############################################

void Read_x1_TestFn(u64& processedBytes) {
    Read_x1(N_COUNT, g_data);
    processedBytes = N_COUNT;
}
void Read_x2_TestFn(u64& processedBytes) {
    Read_x2(N_COUNT, g_data);
    processedBytes = N_COUNT;
}
void Read_x3_TestFn(u64& processedBytes) {
    Read_x3(N_COUNT, g_data);
    processedBytes = N_COUNT;
}
void Read_x4_TestFn(u64& processedBytes) {
    Read_x4(N_COUNT, g_data);
    processedBytes = N_COUNT;
}
void Read_1x2_TestFn(u64& processedBytes) {
    Read_1x2(N_COUNT, g_data);
    processedBytes = N_COUNT;
}

void Write_x1_TestFn(u64& processedBytes) {
    Write_x1(N_COUNT, g_data);
    processedBytes = N_COUNT;
    verifyWrite();
}
void Write_x2_TestFn(u64& processedBytes) {
    Write_x2(N_COUNT, g_data);
    processedBytes = N_COUNT;
    verifyWrite();
}
void Write_x3_TestFn(u64& processedBytes) {
    Write_x3(N_COUNT, g_data);
    processedBytes = N_COUNT;
    verifyWrite();
}
void Write_x4_TestFn(u64& processedBytes) {
    Write_x4(N_COUNT, g_data);
    processedBytes = N_COUNT;
    verifyWrite();
}
void Write_1x2_TestFn(u64& processedBytes) {
    Write_1x2(N_COUNT, g_data);
    processedBytes = N_COUNT;
    Assert(g_data[0] == 0xFF);
}

// ###################################################### MAIN #########################################################

i32 main() {
    coreInit();

    RepetitionTester tester = {};

    // Skylake has 2 ports for reading:
    tester.registerFn(Read_x1_TestFn, FN_NAME_TO_CPTR(Read_x1_TestFn));
    tester.registerFn(Read_x2_TestFn, FN_NAME_TO_CPTR(Read_x2_TestFn));
    tester.registerFn(Read_x3_TestFn, FN_NAME_TO_CPTR(Read_x3_TestFn));
    tester.registerFn(Read_x4_TestFn, FN_NAME_TO_CPTR(Read_x4_TestFn));
    tester.registerFn(Read_1x2_TestFn, FN_NAME_TO_CPTR(Read_1x2_TestFn));

    // Skylake has only 1 port for writing:
    tester.registerFn(Write_x1_TestFn, FN_NAME_TO_CPTR(Write_x1_TestFn), [](){ cleanBuffer(); });
    tester.registerFn(Write_x2_TestFn, FN_NAME_TO_CPTR(Write_x2_TestFn), [](){ cleanBuffer(); });
    tester.registerFn(Write_x3_TestFn, FN_NAME_TO_CPTR(Write_x3_TestFn), [](){ cleanBuffer(); });
    tester.registerFn(Write_x4_TestFn, FN_NAME_TO_CPTR(Write_x4_TestFn), [](){ cleanBuffer(); });
    tester.registerFn(Write_1x2_TestFn, FN_NAME_TO_CPTR(Write_1x2_TestFn), [](){ cleanBuffer(); });

    logInfo("Repetition Testing:\n");
    tester.run(10);

    return 0;
}
