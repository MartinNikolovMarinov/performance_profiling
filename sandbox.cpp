#include <core_init.h>
#include <os_metrics.h>

PRAGMA_WARNING_SUPPRESS_ALL

struct Buffer {
    addr_size count;
    u8* data;
};

void writeToAllBytes(Buffer dest) {
    for (u64 i = 0; i < dest.count; i++) {
        dest.data[i] = u8(i);
    }
}

i32 main() {
    coreInit();

    Buffer buff;
    buff.count = 100000;
    buff.data = reinterpret_cast<u8*>(core::getAllocator(core::DEFAULT_ALLOCATOR_ID).alloc(buff.count, 1));
    writeToAllBytes(buff);

    return 0;
}
