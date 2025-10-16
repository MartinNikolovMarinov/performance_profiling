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
extern "C" void Read_8x2(u64 count, u8* out);

// ##################################################### HELPERS #######################################################

constexpr addr_size N_COUNT = 1000000000;
u8 g_data[N_COUNT] = {};

void cleanBuffer() {
    core::memset(g_data, u8(0), N_COUNT);
}

void verifyWrite() {
    for (addr_size i = 0; i < N_COUNT; i++) {
        AssertFmt(g_data[i] == 1, "verifyWrite failed at idx={}", i);
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
void Read_8x2_TestFn(u64& processedBytes) {
    Read_8x2(N_COUNT, g_data);
    processedBytes = N_COUNT;
}

// ###################################################### MAIN #########################################################

i32 main() {
    coreInit();

    RepetitionTester tester = {};

    tester.registerFn(Read_x1_TestFn, FN_NAME_TO_CPTR(Read_x1_TestFn));
    tester.registerFn(Read_x2_TestFn, FN_NAME_TO_CPTR(Read_x2_TestFn));
    tester.registerFn(Read_x3_TestFn, FN_NAME_TO_CPTR(Read_x3_TestFn));
    tester.registerFn(Read_x4_TestFn, FN_NAME_TO_CPTR(Read_x4_TestFn));
    tester.registerFn(Read_1x2_TestFn, FN_NAME_TO_CPTR(Read_1x2_TestFn));

    logInfo("Repetition Testing:\n");
    tester.run(10);

    return 0;
}
