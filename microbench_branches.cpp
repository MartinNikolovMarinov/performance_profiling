#include <core_init.h>
#include <platform.h>
#include <repetition_tester.h>

#include <ctime>

PRAGMA_WARNING_SUPPRESS_ALL

// ############################################### FUNCTIONS TO PROFILE ################################################

extern "C" void ConditionalNOP(u64 count, u8* data);

// ##################################################### HELPERS #######################################################

constexpr addr_size N_COUNT = 10000000;
static u8 g_data[N_COUNT] = {};

// ###################################################### MAIN #########################################################

// i32 a;
// Assert(readOSRandomType(a));
// logInfo("a={}", a);

void ConditionalNOP_NeverTaken_BeforeFn() {
    core::memset(g_data, u8(0), N_COUNT);
}
void ConditionalNOP_NeverTaken_TestFn(u64& processedBytes) {
    ConditionalNOP(N_COUNT, g_data);
    processedBytes = N_COUNT;
}

void ConditionalNOP_AlwaysTaken_BeforeFn() {
    core::memset(g_data, u8(1), N_COUNT);
}
void ConditionalNOP_AlwaysTaken_TestFn(u64& processedBytes) {
    ConditionalNOP(N_COUNT, g_data);
    processedBytes = N_COUNT;
}

void ConditionalNOP_TakeEvery2_BeforeFn() {
    for (addr_size i = 0; i < N_COUNT; i++)
        g_data[i] = (i % 2 == 0) ? 0 : 1;
}
void ConditionalNOP_TakeEvery2_TestFn(u64& processedBytes) {
    ConditionalNOP(N_COUNT, g_data);
    processedBytes = N_COUNT;
}

void ConditionalNOP_TakeEvery3_BeforeFn() {
    for (addr_size i = 0; i < N_COUNT; i++)
        g_data[i] = (i % 3 == 0) ? 0 : 1;
}
void ConditionalNOP_TakeEvery3_TestFn(u64& processedBytes) {
    ConditionalNOP(N_COUNT, g_data);
    processedBytes = N_COUNT;
}

void ConditionalNOP_TakeEvery4_BeforeFn() {
    for (addr_size i = 0; i < N_COUNT; i++)
        g_data[i] = (i % 4 == 0) ? 0 : 1;
}
void ConditionalNOP_TakeEvery4_TestFn(u64& processedBytes) {
    ConditionalNOP(N_COUNT, g_data);
    processedBytes = N_COUNT;
}

void ConditionalNOP_STDRandom_BeforeFn() {
    for (addr_size i = 0; i < N_COUNT; i++)
        g_data[i] = (std::rand() & 1) ? 1 : 0; // random 0 or 1
}
void ConditionalNOP_STDRandom_TestFn(u64& processedBytes) {
    ConditionalNOP(N_COUNT, g_data);
    processedBytes = N_COUNT;
}

void ConditionalNOP_Random_BeforeFn() {
    for (addr_size i = 0; i < N_COUNT; i++) {
        u8 rnd;
        Assert(readOSRandomType(rnd));
        g_data[i] = (rnd & 1) ? 1 : 0; // random 0 or 1
    }
}
void ConditionalNOP_Random_TestFn(u64& processedBytes) {
    ConditionalNOP(N_COUNT, g_data);
    processedBytes = N_COUNT;
}

i32 main() {
    coreInit();
    std::srand((unsigned)std::time(nullptr));

    RepetitionTester tester = {};

    tester.registerFn(ConditionalNOP_NeverTaken_TestFn,
                      FN_NAME_TO_CPTR(ConditionalNOP_NeverTaken_TestFn),
                      ConditionalNOP_NeverTaken_BeforeFn);
    tester.registerFn(ConditionalNOP_AlwaysTaken_TestFn,
                      FN_NAME_TO_CPTR(ConditionalNOP_AlwaysTaken_TestFn),
                      ConditionalNOP_AlwaysTaken_BeforeFn);
    tester.registerFn(ConditionalNOP_TakeEvery2_TestFn,
                      FN_NAME_TO_CPTR(ConditionalNOP_TakeEvery2_TestFn),
                      ConditionalNOP_TakeEvery2_BeforeFn);
    tester.registerFn(ConditionalNOP_TakeEvery3_TestFn,
                      FN_NAME_TO_CPTR(ConditionalNOP_TakeEvery3_TestFn),
                      ConditionalNOP_TakeEvery3_BeforeFn);
    tester.registerFn(ConditionalNOP_TakeEvery4_TestFn,
                      FN_NAME_TO_CPTR(ConditionalNOP_TakeEvery4_TestFn),
                      ConditionalNOP_TakeEvery4_BeforeFn);
    tester.registerFn(ConditionalNOP_STDRandom_TestFn,
                      FN_NAME_TO_CPTR(ConditionalNOP_STDRandom_TestFn),
                      ConditionalNOP_STDRandom_BeforeFn);
    tester.registerFn(ConditionalNOP_Random_TestFn,
                      FN_NAME_TO_CPTR(ConditionalNOP_Random_TestFn),
                      ConditionalNOP_Random_BeforeFn);

    logInfo("Repetition Testing:\n");
    tester.run(10);

    return 0;
}
