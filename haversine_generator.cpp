#include <core_init.h>
#include <haversine_distance.h>

enum struct RandomPairGenerationMethod : u8 {
    None,
    Uniform,
    Clustered
};

struct Pair {
    f64 x0, y0, x1, y1;
};

void generateRandomUniformPairs(Pair* pairs, f64* results, addr_size pCount) {
    logInfo("Generating {} random uniform pairs", pCount);

    f64 sum = 0;
    f64 sumCoef = 1.0 / f64(pCount);

    for (addr_size i = 0; i < pCount; i++) {
        Pair& pair = pairs[i];

        pair.x0 = core::rndF64(-180.0, 180.0);
        pair.y0 = core::rndF64(-90.0, 90.0);
        pair.x1 = core::rndF64(-180.0, 180.0);
        pair.y1 = core::rndF64(-90.0, 90.0);

        f64 haversineDistance = referenceHaversine(pair.x0, pair.y0, pair.x1, pair.y1, EARTH_RADIUS_KM);
        results[i] = haversineDistance;
        sum += sumCoef * haversineDistance;

        if (core::getLogLevel() <= core::LogLevel::L_DEBUG) {
            core::logDirectStd("Calculated Haversine Distance = ");
            logDirectStdF64(haversineDistance);
            core::logDirectStd("\n");
        }
    }

    if (core::getLogLevel() <= core::LogLevel::L_INFO) {
        core::logDirectStd("Expected sum: ");
        logDirectStdF64(sum);
        core::logDirectStd("\n");
    };
}

void generateRandomClusteredPairs(Pair* pairs, f64* results, addr_size pCount) {
    f64 sum = 0;
    f64 sumCoef = 1.0 / f64(pCount);

    logTrace("Generating {} random clustered pairs", pCount);

    constexpr u32 CLUSTER_COUNT = 64;
    struct Cluster { f64 x, y, r; };
    Cluster clusters[CLUSTER_COUNT];

    auto handleOverflows = [](f64& x, f64& y) {
        if (x < -180.0) x += 360.0;
        if (x > 180.0) x -= 360.0;
        if (y < -90.0) y += 180.0;
        if (y > 90.0) y -= 180.0;
    };

    for (u32 i = 0; i < CLUSTER_COUNT; i++) {
        clusters[i].x = core::rndF64(-180.0, 180.0);
        clusters[i].y = core::rndF64(-90.0, 90.0);
        clusters[i].r = core::rndF64(10.0, 50.0);

        logTrace("Cluster {}: ({:f.16}, {:f.16}) r={:f.16}", i, clusters[i].x, clusters[i].y, clusters[i].r);
    }

    for (addr_size i = 0; i < pCount; i++) {
        Pair& pair = pairs[i];
        Cluster& cluster = clusters[i % CLUSTER_COUNT];

        pair.x0 = core::rndF64(cluster.x - cluster.r, cluster.x + cluster.r);
        pair.y0 = core::rndF64(cluster.y - cluster.r, cluster.y + cluster.r);
        pair.x1 = core::rndF64(cluster.x - cluster.r, cluster.x + cluster.r);
        pair.y1 = core::rndF64(cluster.y - cluster.r, cluster.y + cluster.r);

        handleOverflows(pair.x0, pair.y0);
        handleOverflows(pair.x1, pair.y1);

        f64 haversineDistance = referenceHaversine(pair.x0, pair.y0, pair.x1, pair.y1, EARTH_RADIUS_KM);
        results[i] = haversineDistance;
        sum += sumCoef * haversineDistance;

        if (core::getLogLevel() <= core::LogLevel::L_DEBUG) {
            core::logDirectStd("Calculated Haversine Distance = ");
            logDirectStdF64(haversineDistance);
            core::logDirectStd("\n");
        }
    }

    if (core::getLogLevel() <= core::LogLevel::L_INFO) {
        core::logDirectStd("Expected sum: ");
        logDirectStdF64(sum);
        core::logDirectStd("\n");
    }
}

void toJson(core::StrBuilder<>& sb, const Pair pair) {
    using namespace core;

    constexpr addr_size bufferMax = 254;
    char buff[bufferMax] = {};
    char* start = buff;
    char* curr = buff;

    curr += core::memcopy(curr, "{\"x0\":", core::cstrLen("{\"x0\":"));
    curr += core::Unpack(core::floatToCstr(pair.x0, curr, bufferMax));
    curr += core::memcopy(curr, ", \"y0\":", core::cstrLen(", \"y0\":"));
    curr += core::Unpack(core::floatToCstr(pair.y0, curr, bufferMax));
    curr += core::memcopy(curr, ", \"x1\":", core::cstrLen(", \"x1\":"));
    curr += core::Unpack(core::floatToCstr(pair.x1, curr, bufferMax));
    curr += core::memcopy(curr, ", \"y1\":", core::cstrLen(", \"y1\":"));
    curr += core::Unpack(core::floatToCstr(pair.y1, curr, bufferMax));
    curr += core::memcopy(curr, "}", core::cstrLen("}"));

    sb.append(buff, addr_size(curr - start));
}

void toJson(core::StrBuilder<>& sb, const Pair* pairs, addr_size pCount) {
    sb.append("[\n"_sv);
    for (addr_size i = 0; i < pCount - 1; i++) {
        toJson(sb, pairs[i]);
        sb.append(",\n"_sv);
    }
    if (pCount - 1 > 0) {
        toJson(sb, pairs[pCount - 1]);
        sb.append('\n');
    }
    sb.append("]\n"_sv);
}

struct CommandLineArguments {
    RandomPairGenerationMethod genMethod;
    u64 seed;
    addr_size pairCount;
    core::StrBuilder<> outFileSb;
};

void printUsage() {
    core::logDirectStd("Usage: ./haversine_generator <uniform/cluster> <random seed> <number of coordinate pairs to generate> [filepath]\n");
    core::logDirectStd("Options:\n");
    core::logDirectStd("  --help: Print this help message\n");
}

CommandLineArguments parseCmdArguments(i32 argc, const char** argv) {
    core::CmdFlagParser flagParser;
    core::Expect(flagParser.parse(addr_size(argc), argv), "Failed to parse command line flags!");

    if (flagParser.argumentCount() < 3) {
        printUsage();
        Panic(false, "Invalid number of arguments!");
    }

    CommandLineArguments cmdArgs;
    bool argumentsAreValid = true;
    flagParser.arguments([&argumentsAreValid, &cmdArgs] (core::StrView value, addr_size idx) -> bool {
        switch (idx) {
            case 0:
            {
                if (value.eq("uniform"_sv)) {
                    cmdArgs.genMethod = RandomPairGenerationMethod::Uniform;
                }
                else if (value.eq("cluster"_sv)) {
                    cmdArgs.genMethod = RandomPairGenerationMethod::Clustered;
                }
                else {
                    logErr("Error: '{}' is an invalid random pair generation method!\n\n", value.data());
                    printUsage();
                    argumentsAreValid = false;
                    return false;
                }

                break;
            }

            case 1:
            {
                cmdArgs.seed = core::Unpack(core::cstrToInt<u64>(value.data(), u32(value.len())));
                break;
            }

            case 2:
            {
                cmdArgs.pairCount =  core::Unpack(core::cstrToInt<addr_size>(value.data(), u32(value.len())));
                if (cmdArgs.pairCount == 0) {
                    logErr("Error: pairs count must be greater than 0!\n\n");
                    printUsage();
                    argumentsAreValid = false;
                    return false;
                }

                break;
            }

            case 3:
            {
                cmdArgs.outFileSb = core::StrBuilder(value);
                break;
            }
        }

        return true;
    });

    Panic(argumentsAreValid, "Invalid input arguments!");

    return cmdArgs;
}

i32 main(i32 argc, const char** argv) {
    coreInit();

    // core::setLoggerLevel(core::LogLevel::L_DEBUG);

    auto cmdArgs = parseCmdArguments(argc, argv);

    core::rndInit(cmdArgs.seed, u32(cmdArgs.seed));

    Pair* pairs = defaultAlloc<Pair>(cmdArgs.pairCount);
    f64* results = defaultAlloc<f64>(cmdArgs.pairCount);
    defer {
        defaultFree(pairs, cmdArgs.pairCount);
        defaultFree(results, cmdArgs.pairCount);
    };

    logInfo("Random seed: {}", cmdArgs.seed);
    logInfo("Pair count: {}", cmdArgs.pairCount);

    if (cmdArgs.genMethod == RandomPairGenerationMethod::Uniform) {
        logInfo("Method: uniform");
        generateRandomUniformPairs(pairs, results, cmdArgs.pairCount);
    }
    else if (cmdArgs.genMethod == RandomPairGenerationMethod::Clustered) {
        logInfo("Method: cluster");
        generateRandomClusteredPairs(pairs, results, cmdArgs.pairCount);
    }
    else {
        Panic(false, "Implementation bug!");
    }

    if (cmdArgs.outFileSb.empty()) {
        cmdArgs.outFileSb = core::sv("pairs.json");
    }

    auto outFileSb = std::move(cmdArgs.outFileSb);
    auto pairCount = cmdArgs.pairCount;
    auto outReferenceResultFileSb = outFileSb.copy();
    outReferenceResultFileSb.append(".res"_sv);

    // Write The Pairs
    {
        core::StrBuilder pairsSb;
        toJson(pairsSb, pairs, pairCount);
        logTrace("Pairs: \n{}\n", pairsSb.view().len(), pairsSb.view().data());
        core::Expect(
            core::fileWriteEntire(outFileSb.view().data(),
                                  reinterpret_cast<const u8*>(pairsSb.view().data()),
                                  pairsSb.len()),
            "Failed to write pairs to file!"
        );

        logInfo("Wrote Pairs To '{}'", outFileSb.view().data());
    }

    // Write The Distances For Each Pair
    {
        core::Expect(
            core::fileWriteEntire(outReferenceResultFileSb.view().data(),
                                  reinterpret_cast<const u8*>(results),
                                  pairCount * sizeof(f64)),
            "Failed to write pairs to file!"
        );

        logInfo("Wrote Results To '{}'", outReferenceResultFileSb.view().data());
    }

    // Read Distances Back To Verify
    if (core::getLogLevel() <= core::LogLevel::L_DEBUG) {
        core::ArrList<u8> readbackResults;
        core::Expect(
            core::fileReadEntire(outReferenceResultFileSb.view().data(), readbackResults),
            "Failed to write pairs to file!"
        );

        for (addr_size i = 0; i < readbackResults.len(); i+=sizeof(f64)) {
            f64* v = reinterpret_cast<f64*>(readbackResults.data() + i); // Do not use [i] ! it's obvious why..
            if (core::getLogLevel() <= core::LogLevel::L_DEBUG) {
                logDirectStdF64(*v);
                core::logDirectStd("\n");
            }
        }
    }

    return 0;
}
