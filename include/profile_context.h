#include <core_init.h>

// IMPORTANT: Don't do this nonsense for anything serious!
//            Depending on context management at the call sight is very prone to errors.
//            This approach has serious drawbacks when profiling nested context dependant functions.

struct ProfileContext {
    u32 point;
    const char* label;
};

core::Profiler g_profiler;
core::ArrStatic<ProfileContext, 100> g_pctx;

#define P_SET_SCOPED_CTX(...)                   \
    defer { g_pctx.clear(); };                  \
    ProfileContext pairs[] = { __VA_ARGS__ };   \
    for (auto& pair : pairs) g_pctx.push(pair);

#define P_SET_CTX(...)                          \
    ProfileContext pairs[] = { __VA_ARGS__ };   \
    for (auto& pair : pairs) g_pctx.push(pair);

#define P_CLEAR_CTX() g_pctx.clear();

#define P_TIME_BLOCK(idx)             TIME_BLOCK(g_profiler, g_pctx[(idx)].point, g_pctx[(idx)].label);
#define P_THROUGHPUT_BLOCK(idx, size) THROUGHPUT_BLOCK(g_profiler, g_pctx[(idx)].point, g_pctx[(idx)].label, size);
