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

f64 square(f64 x) { return x*x; }

f64 referenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 earthRadius) {
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;

    f64 dLat = core::degToRad(lat2 - lat1).value;
    f64 dLon = core::degToRad(lon2 - lon1).value;
    lat1 = core::degToRad(lat1).value;
    lat2 = core::degToRad(lat2).value;

    f64 a = square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));

    f64 Result = earthRadius * c;

    return Result;
}

struct Pair {
    f64 x0, y0, x1, y1;
};

void serializePairs(Pair* pairs, addr_size pCount, const char* filepath) {
    // printf("Serializing %zu pairs to %s\n", pCount, filepath);

    core::FileDesc fd =
        ValueOrDie(
            core::fileOpen(filepath,
                core::OpenMode::Read |
                core::OpenMode::Write |
                core::OpenMode::Create |
                core::OpenMode::Truncate),
            "Failed to open file for writing"
        );

    ValueOrDie(
        core::fileWrite(fd, "{\"pairs\":[\n", core::cptrLen("{\"pairs\":[\n"))
    );

    f64 hAverage = 0.0;

    constexpr addr_size FILE_BUFFER_SIZE = core::KILOBYTE * 4;
    char fileBuffer[FILE_BUFFER_SIZE];
    char* fileBufferPtr = fileBuffer;
    constexpr addr_size LINE_BUFFER_SIZE = 256;
    char lineBuffer[LINE_BUFFER_SIZE];

    for (addr_size i = 0; i < pCount; i++) {
        Pair& pair = pairs[i];

        lineBuffer[0] = 0;
        i32 n = sprintf(lineBuffer,
            "\t{\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f },\n",
            pair.x0, pair.y0, pair.x1, pair.y1);

        addr_size maxWrite = fileBuffer + FILE_BUFFER_SIZE - fileBufferPtr;

        if (maxWrite >= addr_size(n)) {
            fileBufferPtr = core::cptrCopy(fileBufferPtr, lineBuffer, n);
        }
        else {
            // The buffer is not large enough to hold the entire line

            // Fill the buffer to capacity:
            core::cptrCopy(fileBufferPtr, lineBuffer, maxWrite);

            // Flush the buffer to disk:
            ValueOrDie(core::fileWrite(fd, fileBuffer, FILE_BUFFER_SIZE));

            // Reset the buffer pointer to the beginning:
            fileBufferPtr = fileBuffer;

            // Copy the remaining characters:
            fileBufferPtr = core::cptrCopy(fileBufferPtr, lineBuffer + maxWrite, n - maxWrite);
        }

        // Null terminate the buffer:
        *fileBufferPtr = 0;

        // Calculate the reference distance
        hAverage += referenceHaversine(pair.x0, pair.y0, pair.x1, pair.y1, EARTH_RADIUS);

        // Debug print:
        // printf("%s", lineBuffer);
    }

    if (fileBufferPtr != fileBuffer) {
        // Flush the buffer to disk:
        ValueOrDie(core::fileWrite(fd, fileBuffer, fileBufferPtr - fileBuffer));
    }

    hAverage = hAverage / pCount;
    printf("Average Haversine distance: %.16f\n", hAverage);

    ValueOrDie(
        core::fileWrite(fd, "]}\n", core::cptrLen("]}\n"))
    );
}

void generateRandomUniformPairs(Pair* pairs, addr_size pCount) {
    // printf("Generating %zu random uniform pairs\n", pCount);

    for (addr_size i = 0; i < pCount; i++) {
        Pair& pair = pairs[i];

        pair.x0 = core::rndF64(-180.0, 180.0);
        pair.y0 = core::rndF64(-90.0, 90.0);
        pair.x1 = core::rndF64(-180.0, 180.0);
        pair.y1 = core::rndF64(-90.0, 90.0);
    }
}

void generateRandomClusteredPairs(Pair* pairs, addr_size pCount) {
    // printf("Generating %zu random clustered pairs\n", pCount);

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

        // printf("Cluster %u: (%.16f, %.16f) r=%.16f\n", i, clusters[i].x, clusters[i].y, clusters[i].r);
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

enum struct RandomPairGenerationMethod : u8 {
    None,
    Uniform,
    Clustered
};

struct CommandLineArguments {
    RandomPairGenerationMethod genMethod;
    u64 seed;
    addr_size pairCount;
    core::StrBuilder<> outFileSb;
};

void printUsage() {
    printf("Usage: ./haversine_generator [uniform/cluster] [random seed] [number of coordinate pairs to generate]\n");
    printf("Options:\n");
    printf("  --help: Print this help message\n");
    printf("  -o [filepath]: Output file path (optional)\n");
}

bool parserCmdArguments(i32 argc, const char** argv, CommandLineArguments& cmdArgs) {
    cmdArgs = CommandLineArguments{};

    core::CmdFlagParser flagParser;

    Expect(
        flagParser.parse(argc, argv),
        "Failed to parse command line flags!"
    );

    // Set flags:
    flagParser.setFlagString(&cmdArgs.outFileSb, core::sv("o"), false, nullptr);

    // Match flags:
    Expect(flagParser.matchFlags(), "Failed to match command line flags!");

    // Handle help option:
    {
        bool isHelpOption = false;
        flagParser.options([&isHelpOption](core::StrView option) {
            if (option.eq(core::sv("--help"))) {
                printUsage();
                isHelpOption = true;
                return false;
            }
            return true;
        });
        if (isHelpOption) {
            return true;
        }
    }

    // Verify argument count is more than the expected minimum:
    if (flagParser.argumentCount() < 3) {
        fprintf(stderr, "Invalid number of arguments!\n");
        printUsage();
        return false;
    }

    // Set the arguments, and verify they are valid:
    {
        bool argsOk = true;
        flagParser.arguments([&argsOk, &cmdArgs](core::StrView value, addr_size idx) {
            switch (idx) {
                case 0:
                {
                    if (value.eq(core::sv("uniform"))) {
                        cmdArgs.genMethod = RandomPairGenerationMethod::Uniform;
                    }
                    else if (value.eq(core::sv("cluster"))) {
                        cmdArgs.genMethod = RandomPairGenerationMethod::Clustered;
                    }
                    else {
                        fprintf(stderr, "Invalid random pair generation method!\n");
                        printUsage();
                        argsOk = false;
                        return false;
                    }

                    break;
                }

                case 1:
                {
                    cmdArgs.seed = core::cptrToInt<u64>(value.data());
                    break;
                }

                case 2:
                {
                    cmdArgs.pairCount = core::cptrToInt<addr_size>(value.data());
                    break;
                }

                default:
                {
                    fprintf(stderr, "Invalid number of arguments!\n");
                    printUsage();
                    argsOk = false;
                    return false;
                }
            }

            return true;
        });
        if (!argsOk) {
            return false;
        }
    }

    printf("Method: %s\n", cmdArgs.genMethod == RandomPairGenerationMethod::Uniform ? "Uniform" : "Clustered");
    printf("Random seed: %lu\n", cmdArgs.seed);
    printf("Pair count: %zu\n", cmdArgs.pairCount);

    return true;
}

i32 main(i32 argc, const char** argv) {
    if (!initCore(argc, argv)) {
        printf("%s\n", "Failed to initialize core!");
        return -1;
    }

    CommandLineArguments cmdArgs;
    if (!parserCmdArguments(argc, argv, cmdArgs)) {
        return -2;
    }

    core::rndInit(cmdArgs.seed);

    Pair* pairs = dalloc<Pair>(cmdArgs.pairCount);
    defer { dfree<Pair>(pairs, cmdArgs.pairCount); };

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
    printf("Output file: %s\n", cmdArgs.outFileSb.view().data());

    serializePairs(pairs, cmdArgs.pairCount, cmdArgs.outFileSb.view().data());

    return 0;
}
