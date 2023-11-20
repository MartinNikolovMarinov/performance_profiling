#include <init_core.h>

constexpr f64 EARTH_RADIOUS = 6372.8;

f64 square(f64 x) { return x*x; }

f64 referenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 earthRadious) {
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;

    f64 dLat = core::deg_to_rad(lat2 - lat1);
    f64 dLon = core::deg_to_rad(lon2 - lon1);
    lat1 = core::deg_to_rad(lat1);
    lat2 = core::deg_to_rad(lat2);

    f64 a = square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));

    f64 Result = earthRadious * c;

    return Result;
}

i32 main(i32 argc, const char** argv) {
    [[maybe_unused]] auto coreCtx = initCore(argc, argv);

    return 0;
}
