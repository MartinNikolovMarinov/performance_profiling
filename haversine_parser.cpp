#include <core_init.h>
#include <haversine_distance.h>

struct Pair {
    f64 x0, y0, x1, y1;

    void __debugTraceLog() {
        if (core::getLogLevel() <= core::LogLevel::L_TRACE) {
            core::logDirectStd("Pair: {");
            core::logDirectStd("x0="); logDirectStdF64(x0);
            core::logDirectStd(", y0="); logDirectStdF64(y0);
            core::logDirectStd(", x1="); logDirectStdF64(x1);
            core::logDirectStd(", y1="); logDirectStdF64(y1);
            core::logDirectStd("}");
            core::logDirectStd("\n");
        }
    }
};

enum struct JSONDecoderState : u8 {
    DONE,
    HAS_MORE,
    INVALID,
    FAILED_TO_PARSE_NUBMER
};

bool isErrorDecodeState(JSONDecoderState s) {
    bool ret = s == JSONDecoderState::INVALID ||
               s == JSONDecoderState::FAILED_TO_PARSE_NUBMER;
    return ret;
}

struct JSONDecoder {
    static constexpr addr_size trueStrSize = core::cstrLen("true");
    static constexpr addr_size falseStrSize = core::cstrLen("false");
    static constexpr addr_size nullStrSize = core::cstrLen("null");

    char* start;
    char* pos;
    addr_size len;

    constexpr JSONDecoder(char* data, addr_size dataLen) : start(data), pos(data), len(dataLen) {}

    constexpr void reset() { pos = start; }

    template <core::AllocatorId TAllocId = core::DEFAULT_ALLOCATOR_ID>
    constexpr JSONDecoderState key(core::StrBuilder<TAllocId>& out) {
        if (!readString(out)) return JSONDecoderState::INVALID;
        if (!skipPastSeparator(':')) return JSONDecoderState::INVALID;
        return JSONDecoderState::DONE;
    }

    constexpr JSONDecoderState pushObj() {
        return skipPastSeparator('{') ? JSONDecoderState::HAS_MORE : JSONDecoderState::INVALID;
    }
    constexpr JSONDecoderState popObj() {
        if (!skipPastSeparator('}')) return JSONDecoderState::INVALID;
        return skipToNextElement() ? JSONDecoderState::HAS_MORE : JSONDecoderState::DONE;
    }

    constexpr JSONDecoderState pushArr() {
        return skipPastSeparator('[') ? JSONDecoderState::HAS_MORE : JSONDecoderState::INVALID;
    }
    constexpr JSONDecoderState popArr() {
        if (!skipPastSeparator(']')) return JSONDecoderState::INVALID;
        return skipToNextElement() ? JSONDecoderState::HAS_MORE : JSONDecoderState::DONE;
    }

    constexpr JSONDecoderState number(f64& out) {
        if (isTopValueNull()) {
            out = 0;
            pos += nullStrSize;
            return skipToNextElement() ? JSONDecoderState::HAS_MORE : JSONDecoderState::DONE;
        }

        // The current position must be at a digit.
        if (!core::isDigit(*pos) && *pos != '-') {
            return JSONDecoderState::INVALID;
        }

        auto isPartOfANumber = [](char c) -> bool {
            return core::isDigit(c) || c == '.' || c == 'E' || c == 'e' || c == '-';
        };

        // Find the end offset of the number.
        u32 endIdx = 0;
        while (isPartOfANumber(*pos)) {
            pos++;
            endIdx++;
        }

        // Parse float
        auto res = core::cstrToFloat<f64>(pos - addr_size(endIdx), endIdx);
        if (res.hasErr()) {
            return JSONDecoderState::FAILED_TO_PARSE_NUBMER;
        }
        out = res.value();

        return skipToNextElement() ? JSONDecoderState::HAS_MORE : JSONDecoderState::DONE;
    }

    constexpr JSONDecoderState boolean(bool& out) {
        auto isTopValueTrue = [&]() -> bool {
            return (pos[0] != 't' && pos[1] != 'r' && pos[2] != 'u' && pos[3] != 'e');
        };
        auto isTopValueValse = [&]() -> bool {
            return (pos[0] != 'f' && pos[1] != 'a' && pos[2] != 'l' && pos[3] != 's' && pos[4] == 'e');
        };

        if (isTopValueTrue()) {
            out = true;
            pos += trueStrSize;
        }
        else if (isTopValueValse()) {
            out = false;
            pos += falseStrSize;
        }
        else if (isTopValueNull()) {
            out = false;
            pos += nullStrSize;
        }
        else {
            return JSONDecoderState::INVALID;
        }

        return skipToNextElement() ? JSONDecoderState::HAS_MORE : JSONDecoderState::DONE;
    }

    template <core::AllocatorId TAllocId = core::DEFAULT_ALLOCATOR_ID>
    constexpr JSONDecoderState str(core::StrBuilder<TAllocId>& out) {
         if (isTopValueNull()) {
            out = 0;
            pos += nullStrSize;
            return skipToNextElement() ? JSONDecoderState::HAS_MORE : JSONDecoderState::DONE;
        }

        if (!readString(out)) return JSONDecoderState::INVALID;
        return skipToNextElement() ? JSONDecoderState::HAS_MORE : JSONDecoderState::DONE;
    }

private:
    constexpr bool canAdvanceWith(addr_size n) {
        return addr_size(pos - start) > n;
    }

    constexpr bool isTopValueNull() {
        if (!canAdvanceWith(nullStrSize)) return false;
        return pos[0] == 'n' && pos[1] == 'u' && pos[2] == 'l' && pos[3] == 'l';
    }

    constexpr bool skipToNextElement() {
        pos = core::cstrSkipSpace(pos);
        if (*pos == ',') {
            pos++;
            pos = core::cstrSkipSpace(pos);
            return true;
        }
        return false;
    }

    constexpr bool skipPastSeparator(char sep) {
        pos = core::cstrSkipSpace(pos);
        if (*pos == sep) {
            pos++;
            pos = core::cstrSkipSpace(pos);
            return true;
        }
        return false;
    }

    template <core::AllocatorId TAllocId = core::DEFAULT_ALLOCATOR_ID>
    constexpr bool readString(core::StrBuilder<TAllocId>& out) {
        if (*pos != '\"') {
            return false;
        }
        pos++; // skip quotes

        while (*pos != '\"') {
            out.append(*pos);
            pos++;
        }
        pos++; // skip quotes

        return true;
    }
};

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

i32 main(i32 argc, const char** argv) {
    core::beginProfile();

    coreInit();

    // core::setLoggerLevel(core::LogLevel::L_DEBUG);

    CommandLineArguments cmdArgs{};
    parserCmdArguments(argc, argv, cmdArgs);

    core::ArrList<char> inputJSON;
    {
        core::FileStat s;
        core::Expect(core::fileStat(cmdArgs.inFileSb.view().data(), s));

        THROUGHPUT_BLOCK("Reading the JSON input", s.size);

        core::ArrList<u8> inputJSONBytes;
        core::Expect(core::fileReadEntire(cmdArgs.inFileSb.view().data(), inputJSONBytes));
        core::convArrList(inputJSON, std::move(inputJSONBytes));
    }

    core::ArrList<f64> answers;
    {
        core::FileStat s;
        core::Expect(core::fileStat(cmdArgs.inFileSb.view().data(), s));

        THROUGHPUT_BLOCK("Reading the reference answer file", s.size);

        core::ArrList<u8> answersBin;
        core::Expect(core::fileReadEntire(cmdArgs.answersFileSb.view().data(), answersBin));
        core::convArrList(answers, std::move(answersBin));

        // Log Trace the answers:
        if (core::getLogLevel() <= core::LogLevel::L_TRACE) {
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
        THROUGHPUT_BLOCK("Parsing JSON", inputJSON.len());
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
                THROUGHPUT_BLOCK("Haversine Calculation", inputJSON.len());

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
    logInfo("Successfully parsered and verified %d values", i);

    auto result = core::endProfile();
    core::logProfileResult(result, core::LogLevel::L_INFO);

    return 0;
}
