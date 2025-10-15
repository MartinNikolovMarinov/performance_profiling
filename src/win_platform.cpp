#include <platform.h>

#include <windows.h>
#include <psapi.h>

namespace {

HANDLE g_processHandle = nullptr;

void initMetrics() {
    if (g_processHandle == nullptr) {
        g_processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                      false,
                                      GetCurrentProcessId());
        Panic(g_processHandle != nullptr, "Failed to get the handle to the current process!");
    }
}

} // namespace

u64 readOSPageFaultCount() {
    initMetrics();

    PROCESS_MEMORY_COUNTERS_EX memoryCounters = {};
    memoryCounters.cb = sizeof(memoryCounters);
    GetProcessMemoryInfo(g_processHandle,
                         reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memoryCounters),
                         sizeof(memoryCounters));

    u64 result = memoryCounters.pageFaultCount;
    return result;
}

bool readOSRandomBytes(u64 count, void* dest) {
    Panic(false, "Not implemented yet");
}
