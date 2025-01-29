#pragma once

#include <core_init.h>

constexpr f64 EARTH_RADIUS_KM = 6372.8;

f64 referenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 earthRadius = EARTH_RADIUS_KM);
