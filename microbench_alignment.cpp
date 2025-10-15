#include <core_init.h>
#include <platform.h>
#include <repetition_tester.h>

PRAGMA_WARNING_SUPPRESS_ALL

// ############################################### FUNCTIONS TO PROFILE ################################################

extern "C" void NOPAligned64(u64 count);
extern "C" void NOPAligned1(u64 count);
extern "C" void NOPAligned15(u64 count);
extern "C" void NOPAligned31(u64 count);
extern "C" void NOPAligned63(u64 count);

// ##################################################### HELPERS #######################################################

constexpr addr_size N_COUNT = 1000000000;

void NOPAligned64_TestFn(u64& processedBytes) {
    NOPAligned64(N_COUNT);
    processedBytes = N_COUNT;
}
void NOPAligned1_TestFn(u64& processedBytes) {
    NOPAligned1(N_COUNT);
    processedBytes = N_COUNT;
}
void NOPAligned15_TestFn(u64& processedBytes) {
    NOPAligned15(N_COUNT);
    processedBytes = N_COUNT;
}
void NOPAligned31_TestFn(u64& processedBytes) {
    NOPAligned31(N_COUNT);
    processedBytes = N_COUNT;
}
void NOPAligned63_TestFn(u64& processedBytes) {
    NOPAligned63(N_COUNT);
    processedBytes = N_COUNT;
}

// ###################################################### MAIN #########################################################

i32 main() {
    coreInit();

    RepetitionTester tester = {};

    tester.registerFn(NOPAligned64_TestFn, FN_NAME_TO_CPTR(NOPAligned64_TestFn));
    tester.registerFn(NOPAligned1_TestFn, FN_NAME_TO_CPTR(NOPAligned1_TestFn));
    tester.registerFn(NOPAligned15_TestFn, FN_NAME_TO_CPTR(NOPAligned15_TestFn));
    tester.registerFn(NOPAligned31_TestFn, FN_NAME_TO_CPTR(NOPAligned31_TestFn));

    // On Skylake:
    // The function’s entry sequence (xor rax, rax + NOP padding + align 64) crosses a cache line boundary, causing the
    // first loop instructions to span two I-cache lines instead of one. The CPU must fetch both lines from the
    // instruction cache (and possibly L2 or memory) during decoding and prefetching, increasing frontend latency and
    // performance drops predictably by 2x.
    tester.registerFn(NOPAligned63_TestFn, FN_NAME_TO_CPTR(NOPAligned63_TestFn));

    logInfo("Repetition Testing:\n");
    tester.run(10);

    return 0;
}
