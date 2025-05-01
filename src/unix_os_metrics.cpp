#include <os_metrics.h>

#include <sys/time.h>
#include <sys/resource.h>

u64 readOSPageFaultCount() {
    rusage usage = {};
    getrusage(RUSAGE_SELF, &usage);

    // ru_minflt - page reclaims (soft page faults)
    // ru_majflt - page faults (hard page faults)
    u64 result = usage.ru_minflt + usage.ru_majflt;
    return result;
}
