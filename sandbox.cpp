#include "core_logger.h"
#include "core_profiler.h"
#include "core_types.h"
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
    buff.count = core::CORE_GIGABYTE;
    buff.data = reinterpret_cast<u8*>(core::getAllocator(core::DEFAULT_ALLOCATOR_ID).zeroAlloc(buff.count, 1));


    core::Profiler profiler;
    profiler.beginProfile();

    {
        TIME_BLOCK(profiler, 1, "writeToAllBytes");
        writeToAllBytes(buff);
    }

    for (u64 i = 0; i < buff.count; i++) {
        AssertFmt(buff.data[i] == u8(i), "{} != {}", buff.data[i], u8(i));
    }

    auto result = profiler.endProfile();
    result.logResult(core::LogLevel::L_INFO);

    return 0;
}
