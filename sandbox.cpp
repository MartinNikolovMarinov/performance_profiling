#include <core_init.h>
#include <os_metrics.h>
#include <utils.h>

PRAGMA_WARNING_SUPPRESS_ALL

struct RepetitionTestResult {
    u64 testCount;
    u64 totalTime;
    u64 maxTime;
    u64 minTime;
};

struct RepetitionTester {
    enum struct State : u8 {
        Uninitialized,
        Idle,
        Testing,
    };

    State state = State::Uninitialized;
    u64 startTime = 0;
    u32 runForAtLeastNSeconds = 10;
    u64 runForAtLeastTime = 0;
    u64 currentTestTime = 0;
    u64 CPUFrequency = 0;

    i32 openBlockCount = 0;
    i32 closeBlockCount = 0;

    u64 targetProcessedBytes = 0;
    u64 currentTestProcessedBytes = 0;

    RepetitionTestResult result = {};

    void startTestWave(u64 bytesToProcess, u32 runForAtLeastNSeconds = 10) {
        auto currCPUFreq = core::getCPUFrequencyHz();

        if (state == State::Uninitialized) {
            this->CPUFrequency = currCPUFreq;
            this->targetProcessedBytes = bytesToProcess;
            result.minTime = core::limitMax<u64>();
            state = State::Testing;
        }
        else if (state == State::Idle) {
            constexpr u64 CPU_FREQUENCY_EPSILON = 1000;
            Panic(this->targetProcessedBytes == bytesToProcess, "Bytes to process should be the same for each test wave!");
            Panic(this->CPUFrequency - currCPUFreq > CPU_FREQUENCY_EPSILON,
                "CPU frequency changed during test wave!");
            state = State::Testing;
        }

        runForAtLeastTime = u64(runForAtLeastNSeconds) * currCPUFreq;
        startTime = core::getPerfCounter();
    }

    bool isTesting() {
        Panic(u8(state) > u8(State::Uninitialized), "Repetition tester must be initialized. Call startTestWave first.");
        if (state != State::Testing) return false;

        u64 currentTime = core::getPerfCounter();

        if (openBlockCount > 0) {
            Panic(openBlockCount == closeBlockCount, "Unbalanced begin/end");
            Assert(currentTestProcessedBytes == targetProcessedBytes, "Processed byte count missmatch. Algorithm bug!");

            u64 elapsedTime = currentTestTime;
            result.testCount += 1;
            result.totalTime += elapsedTime;
            if (result.maxTime < elapsedTime) {
                result.maxTime = elapsedTime;
            }

            if (core::getLogLevel() <= core::LogLevel::L_DEBUG) {
                logDebug("Current Min and Max:");
                printTimeWithBandwith("\tMin", result.minTime, CPUFrequency, targetProcessedBytes);
                core::logDirectStd("\n");
                printTimeWithBandwith("\tMax", result.maxTime, CPUFrequency, targetProcessedBytes);
                core::logDirectStd("\n");
                printTime("\tTime left:", runForAtLeastTime - (currentTime - startTime), CPUFrequency);
                core::logDirectStd("\n");
            }

            if (result.minTime > elapsedTime) {
                // NOTE: If we find a new best, start iterating again:
                result.minTime = elapsedTime;
                startTime = currentTime;
            }

            // Clear state for next repetition:
            openBlockCount = 0;
            closeBlockCount = 0;
            currentTestTime = 0;
            currentTestProcessedBytes = 0;
        }

        if (currentTime - startTime > runForAtLeastTime) {
            state = State::Idle;
        }

        return true;
    }

    void begin() {
        openBlockCount++;
        currentTestTime -= core::getPerfCounter();
    }

    void end() {
        closeBlockCount++;
        currentTestTime += core::getPerfCounter();
    }

    void countProcessedBytes(u64 processed) {
        currentTestProcessedBytes += processed;
    }

    void printTestWaveResults() {
        printTimeWithBandwith("Min", result.minTime, CPUFrequency, targetProcessedBytes);
        core::logDirectStd("\n");

        printTimeWithBandwith("Max", result.maxTime, CPUFrequency, targetProcessedBytes);
        core::logDirectStd("\n");

        if (result.testCount > 0) {
            printTimeWithBandwith("Avg",
                      f64(result.totalTime) / f64(result.testCount),
                      CPUFrequency,
                      targetProcessedBytes);
            core::logDirectStd("\n");
        }
    }
};

// ------------------------------------------------ Specific test cases ------------------------------------------------

struct InputArgs {
    core::StrView fname;
    void* buffer;
    addr_size bufferSize;
};

using TestFunction = void(*)(RepetitionTester& tester, const InputArgs& args);

struct TestCase {
    core::StrView label;
    TestFunction fn;
    InputArgs args;
};

void readFrontToBack_fread(RepetitionTester& tester, const InputArgs& args) {
    while(tester.isTesting()) {
        void* allocated = nullptr;
        if (!args.buffer) {
            allocated = defaultAlloc<void>(args.bufferSize);
            Panic(allocated, "Failed to allocated buffer!");
        }
        defer { if(allocated) defaultFree(allocated, args.bufferSize); };

        FILE* file = fopen(args.fname.data(), "rb");
        Assert(file, "fread failed");

        tester.begin();
        void* pickedBuffer = allocated ? allocated : args.buffer;
        auto result = fread(pickedBuffer, args.bufferSize, 1, file);
        tester.end();

        Assert(result == 1, "fread failed");

        tester.countProcessedBytes(args.bufferSize);

        fclose(file);
    }
}

#if OS_WIN == 1
  #include <io.h>        // _open, _read, _close
  #include <fcntl.h>     // _O_BINARY, _O_RDONLY
  #define  OPEN_FN  _open
  #define  READ_FN  _read
  #define  CLOSE_FN _close
#else
  #include <fcntl.h>     // open
  #include <unistd.h>    // read, close
  #define  OPEN_FN  open
  #define  READ_FN  read
  #define  CLOSE_FN close
#endif

void readFrontToBack_read(RepetitionTester& tester, const InputArgs& args)
{
    while (tester.isTesting()) {
        void* allocated = nullptr;
        if (!args.buffer) {
            allocated = defaultAlloc<void>(args.bufferSize);
            Panic(allocated, "Failed to allocated buffer!");
        }
        defer { if(allocated) defaultFree(allocated, args.bufferSize); };

        /* open file unbuffered / binary */
        int fd = OPEN_FN(args.fname.data(),
#if OS_WIN == 1
                         _O_RDONLY | _O_BINARY);
#else
                         O_RDONLY);
#endif
        Panic(fd != -1, "open failed");

        /* time the raw read() / _read() */
        tester.begin();
        addr_size totalRead = 0;
        void* pickedBuffer = allocated ? allocated : args.buffer;
        while (totalRead < args.bufferSize) {
            addr_size chunk = args.bufferSize - totalRead;
            auto n = READ_FN(fd,
                             static_cast<char*>(pickedBuffer) + totalRead,
#if OS_WIN == 1
                             static_cast<unsigned int>(chunk)
#else
                             chunk
#endif
            );
            Panic(n >= 0, "read failed");

            tester.countProcessedBytes(n);

            if (n == 0) break; /* EOF */
        }
        tester.end();

        CLOSE_FN(fd);
    }
}

struct CommandLineArguments {
    core::StrBuilder<> inFileSb;
};

void printUsage() {
    core::logDirectStd("Usage: ./haversine [input file]\n");
    core::logDirectStd("Options:\n");
    core::logDirectStd("  --help: Print this help message\n");
}

void parserCmdArguments(i32 argc, const char** argv, CommandLineArguments& cmdArgs) {
    core::CmdFlagParser flagParser;

    Expect(
        flagParser.parse(addr_size(argc), argv),
        "Failed to parse command line arguments!"
    );

    if (flagParser.argumentCount() < 1) {
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

    // core::setLoggerLevel(core::LogLevel::L_DEBUG);

    CommandLineArguments cmdArgs{};
    parserCmdArguments(argc, argv, cmdArgs);

    addr_size fileSize = 0;
    {
        core::FileStat stat;
        core::Expect(core::fileStat(cmdArgs.inFileSb.view().data(), stat), "Failed to stat the input file.");
        fileSize = stat.size;
    }

    void* preallocatedBuffer = defaultAlloc<void>(fileSize);
    Panic(preallocatedBuffer, "Failed to preallocate buffer!");
    defer { defaultFree(preallocatedBuffer, fileSize); };

    RepetitionTester tester;
    TestCase cases[] = {
        {
            "Read pattern: front-to-back, Memory: Preallocated, Function used: fread"_sv,
            readFrontToBack_fread,
            InputArgs { cmdArgs.inFileSb.view(), preallocatedBuffer, fileSize },
        },
        {
            "Read pattern: front-to-back, Memory: Allocated rer each, Function used: fread"_sv,
            readFrontToBack_fread,
            InputArgs { cmdArgs.inFileSb.view(), nullptr, fileSize },
        },
        {
            "Read pattern: front-to-back, Memory: Preallocated, Function used: read"_sv,
            readFrontToBack_read,
            InputArgs { cmdArgs.inFileSb.view(), preallocatedBuffer, fileSize },
        },
        {
            "Read pattern: front-to-back, Memory: Preallocated, Function used: read"_sv,
            readFrontToBack_read,
            InputArgs { cmdArgs.inFileSb.view(), nullptr, fileSize },
        },
    };

    for (const auto& c : cases) {
        logInfo("---- %s ----", c.label.data());
        tester.startTestWave(fileSize);
        c.fn(tester, c.args);
        tester.printTestWaveResults();
    }

    return 0;
}
