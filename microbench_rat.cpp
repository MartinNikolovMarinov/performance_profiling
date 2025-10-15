#include <core_init.h>
#include <platform.h>
#include <repetition_tester.h>

PRAGMA_WARNING_SUPPRESS_ALL

// ############################################### FUNCTIONS TO PROFILE ################################################

extern "C" void RATAdd();
extern "C" void RATMovAdd();

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

void RATAdd_TestFn(u64& processedBytes) {
    RATAdd();
    processedBytes = 1000000000;
}
void RATMovAdd_TestFn(u64& processedBytes) {
    RATMovAdd();
    processedBytes = 1000000000;
}

// ###################################################### MAIN #########################################################

i32 main() {
    coreInit();

    RepetitionTester tester = {};

    // The first test has more instructions but it's slower, explanation on why is in the assembly.
    tester.registerFn(RATAdd_TestFn, FN_NAME_TO_CPTR(RATAdd_TestFn));
    tester.registerFn(RATMovAdd_TestFn, FN_NAME_TO_CPTR(RATMovAdd_TestFn));

    logInfo("Repetition Testing:\n");
    tester.run(10);

    return 0;
}
