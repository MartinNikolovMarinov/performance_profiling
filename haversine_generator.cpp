#include <core_init.h>

enum struct RandomPairGenerationMethod : u8 {
    None,
    Uniform,
    Clustered
};

struct Pair {
    f64 x0, y0, x1, y1;
};

void generateRandomUniformPairs(Pair* pairs, addr_size pCount) {
    logInfo("Generating %zu random uniform pairs", pCount);

    for (addr_size i = 0; i < pCount; i++) {
        Pair& pair = pairs[i];

        pair.x0 = core::rndF64(-180.0, 180.0);
        pair.y0 = core::rndF64(-90.0, 90.0);
        pair.x1 = core::rndF64(-180.0, 180.0);
        pair.y1 = core::rndF64(-90.0, 90.0);
    }
}

void generateRandomClusteredPairs(Pair* pairs, addr_size pCount) {
    logInfo("Generating %zu random clustered pairs", pCount);

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

        logTrace("Cluster %u: (%.16f, %.16f) r=%.16f", i, clusters[i].x, clusters[i].y, clusters[i].r);
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
    }
}

void toJson(core::StrBuilder<>& sb, const Pair pair) {
    using namespace core;

    constexpr addr_size bufferMax = 254;
    char buff[bufferMax] = {};
    char* start = buff;
    char* curr = buff;

    curr = core::memcopy(curr, "{\"x0\":", core::cstrLen("{\"x0\":"));
    curr += core::Unpack(core::floatToCstr(pair.x0, curr, bufferMax));
    curr = core::memcopy(curr, ", \"y0\":", core::cstrLen(", \"y0\":"));
    curr += core::Unpack(core::floatToCstr(pair.y0, curr, bufferMax));
    curr = core::memcopy(curr, ", \"x1\":", core::cstrLen(", \"x1\":"));
    curr += core::Unpack(core::floatToCstr(pair.x1, curr, bufferMax));
    curr = core::memcopy(curr, ", \"y1\":", core::cstrLen(", \"y1\":"));
    curr += core::Unpack(core::floatToCstr(pair.y1, curr, bufferMax));
    curr = core::memcopy(curr, "}", core::cstrLen("}"));

    sb.append(buff, curr - start);
}

void toJson(core::StrBuilder<>& sb, const Pair* pairs, addr_size pCount) {
    sb.append("{\n"_sv);
    for (addr_size i = 0; i < pCount; i++) {
        toJson(sb, pairs[i]);
        sb.append(",\n"_sv);
    }
    sb.append("}\n"_sv);
}

struct CommandLineArguments {
    RandomPairGenerationMethod genMethod;
    u64 seed;
    addr_size pairCount;
    core::StrBuilder<> outFileSb;
};

void printUsage() {
    core::logDirectStd("Usage: ./haversine_generator <uniform/cluster> <random seed> <number of coordinate pairs to generate> [filepath]\n");
}

CommandLineArguments parseCmdArguments(u32 argc, const char** argv) {
    core::CmdFlagParser flagParser;
    core::Expect(flagParser.parse(argc, argv), "Failed to parse command line flags!");

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
                    logErr("Error: '%s' is an invalid random pair generation method!\n\n", value.data());
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

    auto cmdArgs = parseCmdArguments(argc, argv);

    // Initialize the random number generator:
    core::rndInit(cmdArgs.seed, u32(cmdArgs.seed));

    // Allocate memory for the pairs:
    Pair* pairs = reinterpret_cast<Pair*>(core::getAllocator(core::DEFAULT_ALLOCATOR_ID).alloc(cmdArgs.pairCount, sizeof(Pair)));
    defer { core::getAllocator(core::DEFAULT_ALLOCATOR_ID).free(pairs, cmdArgs.pairCount, sizeof(Pair)); };

    if (cmdArgs.genMethod == RandomPairGenerationMethod::Uniform) {
        generateRandomUniformPairs(pairs, cmdArgs.pairCount);
    }
    else if (cmdArgs.genMethod == RandomPairGenerationMethod::Clustered) {
        generateRandomClusteredPairs(pairs, cmdArgs.pairCount);
    }
    else {
        Panic(false, "Implementation bug!");
    }

    if (cmdArgs.outFileSb.empty()) {
        cmdArgs.outFileSb = core::sv("pairs.json");
    }
    logInfo("Output file: %s\n", cmdArgs.outFileSb.view().data());

    core::StrBuilder pairsSb;
    toJson(pairsSb, pairs, cmdArgs.pairCount);
    logInfo("Pairs: \n%s\n", pairsSb.view().data());

    core::Expect(
        core::fileWriteEntire(cmdArgs.outFileSb.view().data(),
                              reinterpret_cast<const u8*>(pairsSb.view().data()),
                              pairsSb.len()),
        "Failed to write pairs to file!"
    );
    logInfo("Successfully wrote pairs to %s\n", cmdArgs.outFileSb.view().data());

    return 0;
}
