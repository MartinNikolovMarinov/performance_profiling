#include <core_init.h>

struct ProfileTimePoint {
    u64 elapsedChildrenTsc;
    u64 elapsedTsc;
    u64 hitCount;
    const char* label;
    constexpr bool isUsed() { return hitCount > 0; }
};

struct ProfileBlock {
    char const *label;
    u64 startTsc;
    u32 timepointIdx;
    u32 parentBlockIdx;
};

struct ProfileResult {
    core::Memory<ProfileTimePoint> timepoints;
    u64 cpuFrequencyHz;
    u64 totalElapsedTsc;
    u64 totalElapsedNs;
};

ProfileTimePoint& getProfileTimePoint(addr_size idx);
void beginProfile();
ProfileResult endProfile();
void logProfileResult(const ProfileResult& result, core::LogLevel logLevel, u8 logTag = 0);

void __setGlobalProfileBlock(u32 idx);
u32 __getGlobalProfileBlock();

#define TIME_BLOCK2(name, idx)                                                                     \
    ProfileBlock CORE_NAME_CONCAT(block, __LINE__);                                                \
    CORE_NAME_CONCAT(block, __LINE__).label = name;                                                \
    CORE_NAME_CONCAT(block, __LINE__).timepointIdx = idx;                                          \
    CORE_NAME_CONCAT(block, __LINE__).startTsc = core::getPerfCounter();                           \
    CORE_NAME_CONCAT(block, __LINE__).parentBlockIdx = __getGlobalProfileBlock();                  \
    __setGlobalProfileBlock(idx);                                                                  \
    defer {                                                                                        \
        u64 elapsedTsc = core::getPerfCounter() - CORE_NAME_CONCAT(block, __LINE__).startTsc;      \
        __setGlobalProfileBlock(CORE_NAME_CONCAT(block, __LINE__).parentBlockIdx);                 \
        auto& parentBlock = getProfileTimePoint(CORE_NAME_CONCAT(block, __LINE__).parentBlockIdx); \
        auto& currentBlock = getProfileTimePoint(idx);                                             \
        parentBlock.elapsedChildrenTsc += elapsedTsc;                                              \
        currentBlock.elapsedTsc += elapsedTsc;                                                     \
        currentBlock.hitCount++;                                                                   \
        currentBlock.label = name;                                                                 \
    };
#define TIME_BLOCK(name) TIME_BLOCK2(name, __COUNTER__ + 1)

#define TIME_FUNCTION TIME_BLOCK(__func__)
