#include <core_init.h>
#include <platform.h>
#include <repetition_tester.h>

PRAGMA_WARNING_SUPPRESS_ALL

// ############################################### FUNCTIONS TO PROFILE ################################################

extern "C" void Read_4x2(u64 count, u8* out);
extern "C" void Read_8x2(u64 count, u8* out);
extern "C" void Read_16x2(u64 count, u8* out);
extern "C" void Read_32x2(u64 count, u8* out);

// ##################################################### HELPERS #######################################################

constexpr addr_size N_COUNT = 1000000000;
u8 g_data[N_COUNT] = {};

void cleanBuffer() {
    core::memset(g_data, u8(0), N_COUNT);
}

// ###################################################### TEST FUNCTIONS ###############################################

void Read_4x2_TestFn(u64& processedBytes) {
    Read_4x2(N_COUNT, g_data);
    processedBytes = N_COUNT;
}
void Read_8x2_TestFn(u64& processedBytes) {
    Read_8x2(N_COUNT, g_data);
    processedBytes = N_COUNT;
}
void Read_16x2_TestFn(u64& processedBytes) {
    Read_16x2(N_COUNT, g_data);
    processedBytes = N_COUNT;
}
void Read_32x2_TestFn(u64& processedBytes) {
    Read_32x2(N_COUNT, g_data);
    processedBytes = N_COUNT;
}

// ###################################################### MAIN #########################################################

i32 main() {
    coreInit();

    RepetitionTester tester = {};

    tester.registerFn(Read_4x2_TestFn, FN_NAME_TO_CPTR(Read_4x2_TestFn));
    tester.registerFn(Read_8x2_TestFn, FN_NAME_TO_CPTR(Read_8x2_TestFn));
    tester.registerFn(Read_16x2_TestFn, FN_NAME_TO_CPTR(Read_16x2_TestFn));
    tester.registerFn(Read_32x2_TestFn, FN_NAME_TO_CPTR(Read_32x2_TestFn));

    logInfo("Repetition Testing:\n");
    tester.run(10);

    return 0;
}
