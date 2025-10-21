#include <core_init.h>
#include <repetition_tester.h>

PRAGMA_WARNING_SUPPRESS_ALL

extern "C" void TestCacheSize(u8* data, u64 mask, u64 count);

constexpr u64 N_COUNT = core::CORE_GIGABYTE;
u8 g_data[N_COUNT] = {};

i32 main() {
    coreInit();

    // On my Skylake Machine (Intel(R) Core(TM) i7-8700K (12) @ 4.70 GHz):
    // CPU Cache (L1): 6x32.00 KiB (D), 6x32.00 KiB (I)
    // CPU Cache (L2): 6x256.00 KiB (U)
    // CPU Cache (L3): 12.00 MiB (U)

    // Interpretation of the updated, fully-paged results:
    //
    // 0x000003FF–0x00007FFF  -> ~245 GB/s     -> L1 cache region. Access pattern and masking keep everything resident in L1.
    // 0x0000FFFF–0x0003FFFF  -> 116 - 81 GB/s -> L2 cache. Throughput drops by roughly half, consistent with 256 KB L2 size.
    // 0x0007FFFF–0x003FFFFF  -> 66 - 64 GB/s  -> L3 cache. Shared LLC now handles the working set; bandwidth flattens.
    // 0x007FFFFF–0x01FFFFFF  -> 57 - 26 GB/s  -> Transition from L3 to main memory. Larger masks exceed LLC capacity.
    // >=0x03FFFFFF           -> ~22 GB/s      -> DRAM plateau. This matches typical single-core Skylake memory bandwidth.
    //
    // NOTE: This masking approach is a convenient, generic way to identify approximate cache drop-off points.
    //       However, it skips too quickly through critical regions between L2 and L3 because each left-shift
    //       doubles the working-set size. That exponential growth means only one or two data points fall within
    //       the L2->L3 transition range. A finer-grained sweep-incrementing the working set in smaller steps near
    //       those boundaries - would reveal more precisely where bandwidth begins to fall and the CPU starts
    //       fetching from the next cache level.

    // IMPORTANT: Touch (write to) every page before testing to ensure the OS actually maps them.
    //            On Linux, untouched anonymous memory is often mapped to a shared "zero page."
    //            Without forcing real page allocations, all reads will hit the same physical page,
    //            producing identical results and completely invalidating cache measurements.
    core::memset(g_data, u8(9), N_COUNT);

    RepetitionTester tester = {};

    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00000000000003FF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00000000000007FF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0000000000000FFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0000000000001FFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x1FFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0000000000003FFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0000000000007FFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x000000000000FFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x000000000001FFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x1FFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x000000000003FFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x000000000007FFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00000000000FFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00000000001FFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x1FFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00000000003FFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00000000007FFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0000000000FFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0000000001FFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x1FFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0000000003FFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0000000007FFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x000000000FFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x000000001FFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x1FFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x000000003FFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x000000007FFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00000000FFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00000001FFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x1FFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00000003FFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00000007FFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0000000FFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0000001FFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x1FFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0000003FFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0000007FFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x000000FFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x000001FFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x1FFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x000003FFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x000007FFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00000FFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00001FFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x1FFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00003FFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00007FFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0000FFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0001FFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x1FFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0003FFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0007FFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x000FFFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x001FFFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x1FFFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x003FFFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FFFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x007FFFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FFFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x00FFFFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFFFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x01FFFFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x1FFFFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x03FFFFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FFFFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x07FFFFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FFFFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x0FFFFFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFFFFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x1FFFFFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x1FFFFFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x3FFFFFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x3FFFFFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0x7FFFFFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0x7FFFFFFFFFFFFFFF");
    tester.registerFn([](u64& processedBytes) { TestCacheSize(g_data, 0xFFFFFFFFFFFFFFFF, N_COUNT/128); processedBytes = N_COUNT; }, "Mask 0xFFFFFFFFFFFFFFFF");

    logInfo("Repetition Testing:\n");
    tester.run(50);

    return 0;
}
