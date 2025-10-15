#include <core_init.h>
#include <platform.h>
#include <repetition_tester.h>

PRAGMA_WARNING_SUPPRESS_ALL

// ############################################### FUNCTIONS TO PROFILE ################################################

extern "C" void ConditionalNOP(u64 count, u8* data);

// ##################################################### HELPERS #######################################################

constexpr addr_size N_COUNT = 100000000;
static u8 g_data[N_COUNT] = {};

// ###################################################### MAIN #########################################################

// i32 a;
// Assert(readOSRandomType(a));
// logInfo("a={}", a);

void ConditionalNOP_TestFn(u64& processedBytes) {
    ConditionalNOP(N_COUNT, g_data);
    // verifyAndClearData(g_data, N_COUNT);
    processedBytes = N_COUNT;
}

i32 main() {
    coreInit();

    RepetitionTester tester = {};

    tester.registerFn(ConditionalNOP_TestFn, "ConditionalNOP");

    logInfo("Repetition Testing:\n");
    tester.run(10);

    return 0;
}
