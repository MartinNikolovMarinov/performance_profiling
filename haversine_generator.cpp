#include <core_init.h>

struct Pair {
    f64 x0, y0, x1, y1;
};

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

// void generateRandomClusteredPairs(Pair* pairs, addr_size pCount) {
//     // printf("Generating %zu random clustered pairs\n", pCount);

//     constexpr u32 CLUSTER_COUNT = 64;
//     struct Cluster { f64 x, y, r; };
//     Cluster clusters[CLUSTER_COUNT];

//     auto handleOverflows = [](f64& x, f64& y) {
//         if (x < -180.0) x += 360.0;
//         if (x > 180.0) x -= 360.0;
//         if (y < -90.0) y += 180.0;
//         if (y > 90.0) y -= 180.0;
//     };

//     for (u32 i = 0; i < CLUSTER_COUNT; i++) {
//         clusters[i].x = core::rndF64(-180.0, 180.0);
//         clusters[i].y = core::rndF64(-90.0, 90.0);
//         clusters[i].r = core::rndF64(10.0, 50.0);

//         // printf("Cluster %u: (%.16f, %.16f) r=%.16f\n", i, clusters[i].x, clusters[i].y, clusters[i].r);
//     }

//     for (addr_size i = 0; i < pCount; i++) {
//         Pair& pair = pairs[i];
//         Cluster& cluster = clusters[i % CLUSTER_COUNT];

//         pair.x0 = core::rndF64(cluster.x - cluster.r, cluster.x + cluster.r);
//         pair.y0 = core::rndF64(cluster.y - cluster.r, cluster.y + cluster.r);
//         pair.x1 = core::rndF64(cluster.x - cluster.r, cluster.x + cluster.r);
//         pair.y1 = core::rndF64(cluster.y - cluster.r, cluster.y + cluster.r);

//         handleOverflows(pair.x0, pair.y0);
//         handleOverflows(pair.x1, pair.y1);
//     }
// }

// enum struct RandomPairGenerationMethod : u8 {
//     None,
//     Uniform,
//     Clustered
// };

// struct CommandLineArguments {
//     RandomPairGenerationMethod genMethod;
//     u64 seed;
//     addr_size pairCount;
//     core::StrBuilder<> outFileSb;
// };

// void printUsage() {
//     printf("Usage: ./haversine_generator [uniform/cluster] [random seed] [number of coordinate pairs to generate]\n");
//     printf("Options:\n");
//     printf("  --help: Print this help message\n");
//     printf("  -o [filepath]: Output file path (optional)\n");
// }

// bool parserCmdArguments(i32 argc, const char** argv, CommandLineArguments& cmdArgs) {
//     cmdArgs = CommandLineArguments{};

//     core::CmdFlagParser flagParser;

//     Expect(
//         flagParser.parse(argc, argv),
//         "Failed to parse command line flags!"
//     );

//     // Set flags:
//     flagParser.setFlagString(&cmdArgs.outFileSb, core::sv("o"), false, nullptr);

//     // Match flags:
//     Expect(flagParser.matchFlags(), "Failed to match command line flags!");

//     // Handle help option:
//     {
//         bool isHelpOption = false;
//         flagParser.options([&isHelpOption](core::StrView option) {
//             if (option.eq(core::sv("--help"))) {
//                 printUsage();
//                 isHelpOption = true;
//                 return false;
//             }
//             return true;
//         });
//         if (isHelpOption) {
//             return true;
//         }
//     }

//     // Verify argument count is more than the expected minimum:
//     if (flagParser.argumentCount() < 3) {
//         fprintf(stderr, "Invalid number of arguments!\n");
//         printUsage();
//         return false;
//     }

//     // Set the arguments, and verify they are valid:
//     {
//         bool argsOk = true;
//         flagParser.arguments([&argsOk, &cmdArgs](core::StrView value, addr_size idx) {
//             switch (idx) {
//                 case 0:
//                 {
//                     if (value.eq(core::sv("uniform"))) {
//                         cmdArgs.genMethod = RandomPairGenerationMethod::Uniform;
//                     }
//                     else if (value.eq(core::sv("cluster"))) {
//                         cmdArgs.genMethod = RandomPairGenerationMethod::Clustered;
//                     }
//                     else {
//                         fprintf(stderr, "Invalid random pair generation method!\n");
//                         printUsage();
//                         argsOk = false;
//                         return false;
//                     }

//                     break;
//                 }

//                 case 1:
//                 {
//                     cmdArgs.seed = core::cptrToInt<u64>(value.data());
//                     break;
//                 }

//                 case 2:
//                 {
//                     cmdArgs.pairCount = core::cptrToInt<addr_size>(value.data());
//                     break;
//                 }

//                 default:
//                 {
//                     fprintf(stderr, "Invalid number of arguments!\n");
//                     printUsage();
//                     argsOk = false;
//                     return false;
//                 }
//             }

//             return true;
//         });
//         if (!argsOk) {
//             return false;
//         }
//     }

//     printf("Method: %s\n", cmdArgs.genMethod == RandomPairGenerationMethod::Uniform ? "Uniform" : "Clustered");
//     printf("Random seed: %lu\n", cmdArgs.seed);
//     printf("Pair count: %zu\n", cmdArgs.pairCount);

//     return true;
// }

i32 main(i32, const char**) {
    coreInit();

    // CommandLineArguments cmdArgs;
    // if (!parserCmdArguments(argc, argv, cmdArgs)) {
    //     return -2;
    // }

    // core::rndInit(cmdArgs.seed);

    // Pair* pairs = dalloc<Pair>(cmdArgs.pairCount);
    // defer { dfree<Pair>(pairs, cmdArgs.pairCount); };

    // if (cmdArgs.genMethod == RandomPairGenerationMethod::Uniform) {
    //     generateRandomUniformPairs(pairs, cmdArgs.pairCount);
    // }
    // else if (cmdArgs.genMethod == RandomPairGenerationMethod::Clustered) {
    //     generateRandomClusteredPairs(pairs, cmdArgs.pairCount);
    // }
    // else {
    //     Panic(false, "Implementation bug!");
    // }

    // if (cmdArgs.outFileSb.empty()) {
    //     cmdArgs.outFileSb = core::sv("pairs.json");
    // }
    // printf("Output file: %s\n", cmdArgs.outFileSb.view().data());

    // pairsToJSON(pairs, cmdArgs.pairCount, cmdArgs.outFileSb.view().data());

    // return 0;
}
