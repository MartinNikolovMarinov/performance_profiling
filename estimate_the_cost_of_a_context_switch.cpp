#include "core_types.h"
#include "repetition_tester.h"

#include <core_init.h>

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sched.h>

PRAGMA_WARNING_SUPPRESS_ALL

RepetitionTester tester = {};
u64 estimatedCPUFrequency = core::getCPUFrequencyHz();
i32 pipefds[2];

constexpr i32 numberOfOps = 1;
const char bytesToSend[] = { '1' };
constexpr addr_size bytesToSendLen = CORE_C_ARRLEN(bytesToSend);
char resvBuff[bytesToSendLen + 1] = {};

// ############################################### FUNCTIONS TO PROFILE ################################################

bool Child_TestFn(u64& processedBytes) {
    i32 i = numberOfOps;
    while (i-- > 0) {
        addr_off ret = write(pipefds[1], bytesToSend, bytesToSendLen);
        if (ret < 0) {
            perror("failed to write to pipe");
            exit(-1);
        }
    }
    processedBytes = bytesToSendLen;
    return false;
}

bool Parent_TestFn(u64& processedBytes) {
    i32 i = numberOfOps;
    while (i-- > 0) {
        addr_off ret = read(pipefds[0], resvBuff, bytesToSendLen);
        if (ret < 0) {
            perror("failed to read from pipe");
            exit(-1);
        }
    }

    processedBytes = bytesToSendLen;

    if (resvBuff[0] == '0') {
        // exit signal received
        return true; // stop
    }

    return false;
}

// TODO: Use a futex instead of a pipe to eliminate kernel pipe buffer overhead from the measurement.

// ###################################################### MAIN #########################################################

i32 main() {
    coreInit();

    Assert(geteuid() == 0, "Must be sudo");

    i32 retPipe = pipe(pipefds);
    if (retPipe < 0) {
        perror("Failed to create pipe");
        return -1;
    }
    defer {
        close(pipefds[0]);
        close(pipefds[1]);
    };

    // Pin the current process (and its future child) to CPU 0,
    // so that pipe communication forces a context switch on the same core.
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
        perror("Failed to set CPU affinity");
        return -1;
    }

    // Use SCHED_FIFO so the scheduler immediately switches when one process blocks.
    sched_param sp = {};
    sp.sched_priority = 1; // minimum real-time priority
    if (sched_setscheduler(0, SCHED_FIFO, &sp) != 0) {
        perror("Failed to set SCHED_FIFO (requires root)");
        return -1;
    }

    i32 fpid = fork();
    if (fpid < 0) {
        perror("Failed to fork process");
        return -1;
    }

    if (fpid == 0) {
        // Child
        tester.registerFn(Child_TestFn, FN_NAME_TO_CPTR(Child_TestFn));
        tester.run(20);
        logInfo("Child Done");

        // Signal to the parent that it should exit.
        addr_off ret = write(pipefds[1], "0", 1);
        if (ret < 0) {
            perror("failed to write exit signal to pipe");
            exit(-1);
        }
    }
    else {
        // Parent
        tester.registerFn(Parent_TestFn, FN_NAME_TO_CPTR(Parent_TestFn));
        tester.run(20);
        logInfo("Parent Done");
    }

    return 0;
}
