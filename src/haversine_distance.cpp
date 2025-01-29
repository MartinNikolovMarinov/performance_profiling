#include <haversine_distance.h>

namespace {

f64 square(f64 A) {
    f64 Result = (A*A);
    return Result;
}

f64 radiansFromDegrees(f64 Degrees) {
    f64 Result = 0.01745329251994329577 * Degrees;
    return Result;
}

}

f64 referenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 earthRadius) {
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;

    f64 dLat = radiansFromDegrees(lat2 - lat1);
    f64 dLon = radiansFromDegrees(lon2 - lon1);
    lat1 = radiansFromDegrees(lat1);
    lat2 = radiansFromDegrees(lat2);

    f64 a = square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));

    f64 res = earthRadius * c;

    return res;
}
