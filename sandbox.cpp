#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

typedef uint32_t u32;
typedef uint64_t u64;
typedef double f64;
#define U64Max UINT64_MAX

static f64 Square(f64 A)
{
    f64 Result = (A*A);
    return Result;
}

static f64 RadiansFromDegrees(f64 Degrees)
{
    f64 Result = 0.01745329251994329577 * Degrees;
    return Result;
}

static f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{

    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;

    f64 dLat = RadiansFromDegrees(lat2 - lat1);
    f64 dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);

    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));

    f64 Result = EarthRadius * c;

    return Result;
}

struct random_series
{
    u64 A, B, C, D;
};

static u64 RotateLeft(u64 V, int Shift)
{
    u64 Result = ((V << Shift) | (V >> (64-Shift)));
    return Result;
}

static u64 RandomU64(random_series *Series)
{
    u64 A = Series->A;
    u64 B = Series->B;
    u64 C = Series->C;
    u64 D = Series->D;

    u64 E = A - RotateLeft(B, 27);

    A = (B ^ RotateLeft(C, 17));
    B = (C + D);
    C = (D + E);
    D = (E + A);

    Series->A = A;
    Series->B = B;
    Series->C = C;
    Series->D = D;

    return D;
}

static random_series Seed(u64 Value)
{
    random_series Series = {};

    // NOTE(casey): This is the seed pattern for JSF generators, as per the original post
    Series.A = 0xf1ea5eed;
    Series.B = Value;
    Series.C = Value;
    Series.D = Value;

    u32 Count = 20;
    while(Count--)
    {
        RandomU64(&Series);
    }

    return Series;
}

static f64 RandomInRange(random_series *Series, f64 Min, f64 Max)
{
    f64 t = (f64)RandomU64(Series) / (f64)U64Max;
    f64 Result = (1.0 - t)*Min + t*Max;

    return Result;
}

static FILE *Open(long long unsigned PairCount, char const *Label, char const *Extension)
{
    char Temp[256];
    sprintf(Temp, "data_%llu_%s.%s", PairCount, Label, Extension);
    FILE *Result = fopen(Temp, "wb");
    if(!Result)
    {
        fprintf(stderr, "Unable to open \"%s\" for writing.\n", Temp);
    }

    return Result;
}

static f64 RandomDegree(random_series *Series, f64 Center, f64 Radius, f64 MaxAllowed)
{
    f64 MinVal = Center - Radius;
    if(MinVal < -MaxAllowed)
    {
        MinVal = -MaxAllowed;
    }

    f64 MaxVal = Center + Radius;
    if(MaxVal > MaxAllowed)
    {
        MaxVal = MaxAllowed;
    }

    f64 Result = RandomInRange(Series, MinVal, MaxVal);
    return Result;
}

int main(int ArgCount, char **Args)
{
    if(ArgCount == 4)
    {
        u64 ClusterCountLeft = U64Max;
        f64 MaxAllowedX = 180;
        f64 MaxAllowedY = 90;

        f64 XCenter = 0;
        f64 YCenter = 0;
        f64 XRadius = MaxAllowedX;
        f64 YRadius = MaxAllowedY;

        char const *MethodName = Args[1];
        if(strcmp(MethodName, "cluster") == 0)
        {
            ClusterCountLeft = 0;
        }
        else if(strcmp(MethodName, "uniform") != 0)
        {
            MethodName = "uniform";
            fprintf(stderr, "WARNING: Unrecognized method name. Using 'uniform'.\n");
        }

        u64 SeedValue = atoll(Args[2]);
        random_series Series = Seed(SeedValue);

        u64 MaxPairCount = (1ULL << 34);
        u64 PairCount = atoll(Args[3]);
        if(PairCount < MaxPairCount)
        {
            u64 ClusterCountMax = 1 + (PairCount / 64);

            FILE *FlexJSON = Open(PairCount, "flex", "json");
            FILE *HaverAnswers = Open(PairCount, "haveranswer", "f64");
            if(FlexJSON && HaverAnswers)
            {
                fprintf(FlexJSON, "{\"pairs\":[\n");
                f64 Sum = 0;
                f64 SumCoef = 1.0 / (f64)PairCount;
                for(u64 PairIndex = 0; PairIndex < PairCount; ++PairIndex)
                {
                    if(ClusterCountLeft-- == 0)
                    {
                        ClusterCountLeft = ClusterCountMax;
                        XCenter = RandomInRange(&Series, -MaxAllowedX, MaxAllowedX);
                        YCenter = RandomInRange(&Series, -MaxAllowedY, MaxAllowedY);
                        XRadius = RandomInRange(&Series, 0, MaxAllowedX);
                        YRadius = RandomInRange(&Series, 0, MaxAllowedY);
                    }

                    f64 X0 = RandomDegree(&Series, XCenter, XRadius, MaxAllowedX);
                    f64 Y0 = RandomDegree(&Series, YCenter, YRadius, MaxAllowedY);
                    f64 X1 = RandomDegree(&Series, XCenter, XRadius, MaxAllowedX);
                    f64 Y1 = RandomDegree(&Series, YCenter, YRadius, MaxAllowedY);

                    f64 EarthRadius = 6372.8;
                    f64 HaversineDistance = ReferenceHaversine(X0, Y0, X1, Y1, EarthRadius);

                    Sum += SumCoef*HaversineDistance;

                    char const *JSONSep = (PairIndex == (PairCount - 1)) ? "\n" : ",\n";
                    fprintf(FlexJSON, "    {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f}%s", X0, Y0, X1, Y1, JSONSep);

                    fwrite(&HaversineDistance, sizeof(HaversineDistance), 1, HaverAnswers);
                }
                fprintf(FlexJSON, "]}\n");
                fwrite(&Sum, sizeof(Sum), 1, HaverAnswers);

                fprintf(stdout, "Method: %s\n", MethodName);
                fprintf(stdout, "Random seed: %llu\n", SeedValue);
                fprintf(stdout, "Pair count: %llu\n", PairCount);
                fprintf(stdout, "Expected sum: %.16f\n", Sum);
            }

            if(FlexJSON) fclose(FlexJSON);
            if(HaverAnswers) fclose(HaverAnswers);
        }
        else
        {
            fprintf(stderr, "To avoid accidentally generating massive files, number of pairs must be less than %llu.\n", MaxPairCount);
        }
    }
    else
    {
        fprintf(stderr, "Usage: %s [uniform/cluster] [random seed] [number of coordinate pairs to generate]\n", Args[0]);
    }

    return 0;
}
