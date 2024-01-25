#include <init_core.h>

constexpr f64 EARTH_RADIUS = 6372.8;

template <typename T>
T* dalloc(addr_size count) {
    return reinterpret_cast<T*>(CORE_DEFAULT_ALLOCATOR()::alloc(count * sizeof(T)));
}

template <typename T>
void dfree(void* ptr, addr_size count) {
    CORE_DEFAULT_ALLOCATOR()::free(ptr, count * sizeof(T));
}

constexpr inline f64 square(f64 x) { return x*x; }

f64 referenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 earthRadius);

struct Pair {
    f64 x0, y0, x1, y1;
};

void pairsToJSON(Pair* pairs, addr_size pCount, const char* filepath);
