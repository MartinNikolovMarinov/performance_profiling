#include <init_core.h>
#include <haversine.h>

struct CommandLineArguments {
    f64 answer;
    core::StrBuilder<> inFileSb;
};

void printUsage() {
    printf("Usage: ./haversine [input.json] [expected answer]\n");
    printf("Options:\n");
    printf("  --help: Print this help message\n");
}

bool parserCmdArguments(i32 argc, const char** argv, CommandLineArguments& cmdArgs) {
    core::CmdFlagParser flagParser;

    Expect(
        flagParser.parse(argc, argv),
        "Failed to parse command line arguments!"
    );

    if (flagParser.argumentCount() < 2) {
        fprintf(stderr, "Invalid number of arguments!\n");
        printUsage();
        return false;
    }

    bool argsOk = true;

    flagParser.arguments([&argsOk, &cmdArgs](core::StrView arg, addr_size idx) {
        switch (idx)
        {
            case 0:
                cmdArgs.inFileSb = core::StrBuilder<>(arg);
                break;

            case 1:
                cmdArgs.answer = core::cptrToFloat<f64>(arg.data());
                break;

            default:
                argsOk = false;
                return false;
        }

        return true;
    });

    if (!argsOk) {
        return false;
    }

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

    printf("Input file: %s\n", cmdArgs.inFileSb.view().data());
    printf("Expected answer: %.16f\n", cmdArgs.answer);

    return 0;
}

