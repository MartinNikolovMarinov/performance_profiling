#include <platform.h>

#include <fcntl.h>
#include <errno.h>
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

namespace {
i32 g_urandomFd = -1;
} // namespace


bool readOSRandomBytes(u64 count, void* dest) {
    if (g_urandomFd < 0) {
        g_urandomFd = open("/dev/urandom", O_RDONLY);
        if (g_urandomFd < 0) {
            perror("open(/dev/urandom)");
            return false;
        }
    }

    ssize_t n = read(g_urandomFd, dest, count);
    if (n != ssize_t(count)) {
        perror("read(/dev/urandom)");
        return false;
    }

    return true;
}
