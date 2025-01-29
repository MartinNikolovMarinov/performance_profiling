#include <core_init.h>
 #include <haversine_distance.h>
// #include <json_serializer.h>

struct CommandLineArguments {
    core::StrBuilder<> inFileSb;
    core::StrBuilder<> answersFileSb;
};

void printUsage() {
    core::logDirectStd("Usage: ./haversine [input.json] [expected answer]\n");
    core::logDirectStd("Options:\n");
    core::logDirectStd("  --help: Print this help message\n");
}

void parserCmdArguments(i32 argc, const char** argv, CommandLineArguments& cmdArgs) {
    core::CmdFlagParser flagParser;

    Expect(
        flagParser.parse(argc, argv),
        "Failed to parse command line arguments!"
    );

    if (flagParser.argumentCount() < 2) {
        printUsage();
        Panic(false, "Invalid number of arguments!");
    }

    bool argsOk = true;

    flagParser.arguments([&argsOk, &cmdArgs](core::StrView arg, addr_size idx) {
        switch (idx)
        {
            case 0:
                cmdArgs.inFileSb = core::StrBuilder<>(arg);
                break;

            case 1:
                cmdArgs.answersFileSb = core::StrBuilder<>(arg);
                break;

            default:
                argsOk = false;
                return false;
        }

        return true;
    });

    Panic(argsOk, "Invalid input arguments!");
}

i32 main(i32 argc, const char** argv) {
    coreInit();

    CommandLineArguments cmdArgs{};
    parserCmdArguments(argc, argv, cmdArgs);

    core::ArrList<u8> inputJSON;
    core::Expect(core::fileReadEntire(cmdArgs.inFileSb.view().data(), inputJSON));

    core::ArrList<f64> answers;
    {
        core::ArrList<u8> answersBin;
        core::Expect(core::fileReadEntire(cmdArgs.answersFileSb.view().data(), answersBin));
        core::convArrList(answers, std::move(answersBin));

        for (addr_size i = 0; i < answers.len(); i++) {
            logF64("Answer: %.*s", answers[i], core::LogLevel::L_INFO);
        }
    }

    return 0;
}

