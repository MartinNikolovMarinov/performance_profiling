#include <core_init.h>
#include <haversine_distance.h>
#include <pair.h>
#include <slow_json_decoder.h>

struct CommandLineArguments {
    core::StrBuilder<> inFileSb;
    core::StrBuilder<> answersFileSb;
};

void printUsage() {
    core::logDirectStd("Usage: ./haversine_parser [input.json] [expected answer]\n");
    core::logDirectStd("Options:\n");
    core::logDirectStd("  --help: Print this help message\n");
}

void parserCmdArguments(i32 argc, const char** argv, CommandLineArguments& cmdArgs) {
    core::CmdFlagParser flagParser;

    Expect(
        flagParser.parse(addr_size(argc), argv),
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

core::Profiler profiler;

enum ProfilerPoints {
    PP_RESERVED,

    PP_READING_THE_JSON_INPUT,
    PP_READING_THE_REFERENCE_ANSWER_FILE,
    PP_PARSING_JSON,
    PP_HAVERSINE_CALCULATION,
};

i32 main(i32 argc, const char** argv) {
    profiler.beginProfile();

    coreInit();

    // core::setLoggerLevel(core::LogLevel::L_DEBUG);

    CommandLineArguments cmdArgs{};
    parserCmdArguments(argc, argv, cmdArgs);

    core::ArrList<char> inputJSON;
    {
        core::FileStat s;
        core::Expect(core::fileStat(cmdArgs.inFileSb.view().data(), s));

        THROUGHPUT_BLOCK(profiler, PP_READING_THE_JSON_INPUT, "Reading the JSON input", s.size);

        core::ArrList<u8> inputJSONBytes;
        core::Expect(core::fileReadEntire(cmdArgs.inFileSb.view().data(), inputJSONBytes));
        core::convArrList(inputJSON, std::move(inputJSONBytes));
    }

    core::ArrList<f64> answers;
    {
        core::FileStat s;
        core::Expect(core::fileStat(cmdArgs.answersFileSb.view().data(), s));

        THROUGHPUT_BLOCK(profiler, PP_READING_THE_REFERENCE_ANSWER_FILE, "Reading the reference answer file", s.size);

        core::ArrList<u8> answersBin;
        core::Expect(core::fileReadEntire(cmdArgs.answersFileSb.view().data(), answersBin));
        core::convArrList(answers, std::move(answersBin));

        // Log Trace the answers:
        if (core::loggerGetLevel() <= core::LogLevel::L_TRACE) {
            for (addr_size i = 0; i < answers.len(); i++) {
                core::logDirectStd("Answer: ");
                logDirectStdF64(answers[i]);
                core::logDirectStd("\n");
            }
        }
    }

    JSONDecoder decoder(inputJSON.data(), inputJSON.len());

    core::StrBuilder<> keysb; // TODO: Make this a stack/slab allocator?
    addr_size i = 0;
    {
        THROUGHPUT_BLOCK(profiler, PP_PARSING_JSON, "Parsing JSON", inputJSON.len());
        Panic(decoder.pushArr() == JSONDecoderState::HAS_MORE);

        while (true) {
            Pair p = {};
            f64 number = 0;

            auto matchAndSetKey = [](core::StrView x, f64 v, Pair& out) {
                if (x.eq("x0"_sv)) out.x0 = v;
                else if (x.eq("y0"_sv)) out.y0 = v;
                else if (x.eq("x1"_sv)) out.x1 = v;
                else if (x.eq("y1"_sv)) out.y1 = v;
                else Panic(false, "Pair format is invalid.");
            };

            Panic(decoder.pushObj() == JSONDecoderState::HAS_MORE);

            Panic(decoder.key(keysb) == JSONDecoderState::DONE);
            Panic(decoder.number(number) == JSONDecoderState::HAS_MORE);
            matchAndSetKey(keysb.view(), number, p);
            keysb.clear();

            Panic(decoder.key(keysb) == JSONDecoderState::DONE);
            Panic(decoder.number(number) == JSONDecoderState::HAS_MORE);
            matchAndSetKey(keysb.view(), number, p);
            keysb.clear();

            Panic(decoder.key(keysb) == JSONDecoderState::DONE);
            Panic(decoder.number(number) == JSONDecoderState::HAS_MORE);
            matchAndSetKey(keysb.view(), number, p);
            keysb.clear();

            Panic(decoder.key(keysb) == JSONDecoderState::DONE);
            Panic(decoder.number(number) == JSONDecoderState::DONE);
            matchAndSetKey(keysb.view(), number, p);
            keysb.clear();

            auto state = decoder.popObj();
            Assert(!isErrorDecodeState(state));
            p.__debugTraceLog();

            {
                THROUGHPUT_BLOCK(profiler, PP_HAVERSINE_CALCULATION, "Haversine Calculation", sizeof(p));
                f64 hd = referenceHaversine(p.x0, p.y0, p.x1, p.y1);
                Panic(hd == answers[i], "Did not match expected answer.");
            }

            i++;

            if (state == JSONDecoderState::DONE) {
                break;
            }
        }
        Panic(decoder.popArr() == JSONDecoderState::DONE);
    }

    Panic(i == answers.len(), "Did not calculate the full range of values.");
    logInfo("Successfully parsered and verified {} values", i);

    auto result = profiler.endProfile();
    result.logResult(core::LogLevel::L_INFO);

    return 0;
}
