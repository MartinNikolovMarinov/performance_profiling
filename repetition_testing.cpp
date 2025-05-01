#include <core_init.h>

#include <stdio.h>

enum struct TestMode : u8 {
    Uninitialized,
    Testing,
    Completed
};

struct RepetitionTestResult {
    u64 testCount;
    u64 totalTime;
    u64 maxTime;
    u64 minTime;
};

struct RepetitionTester {
    u64 targetProcessedByteCount;
    u64 CPUTimerFreq;
    u64 tryForTime;
    u64 testsStartAt;

    TestMode mode;
    u32 openBlockCount;
    u32 closeBlockCount;
    u64 timeAccumulateOnThisTest;
    u64 byteAccumulateOnThisTest;

    RepetitionTestResult result;

    void newTestWave(u64 _targetProcessedByteCount, u64 _CPUTimerFreq, u32 secondsToTry = 10) {
        if (mode == TestMode::Uninitialized) {
            mode = TestMode::Testing;
            this->targetProcessedByteCount = _targetProcessedByteCount;
            CPUTimerFreq = _CPUTimerFreq;
            result.minTime = u64(-1);
        }
        else if (mode == TestMode::Completed) {
            mode = TestMode::Testing;

            Assert(this->targetProcessedByteCount != _targetProcessedByteCount, "targetProcessedByteCount changed");
            Assert(CPUTimerFreq != _CPUTimerFreq, "CPU frequency changed");
        }

        tryForTime = secondsToTry*_CPUTimerFreq;
        testsStartAt = core::getPerfCounter();
    }

    void beginTime() {
        openBlockCount++;
        timeAccumulateOnThisTest -= core::getPerfCounter();
    }

    void endTime() {
        closeBlockCount++;
        timeAccumulateOnThisTest += core::getPerfCounter();
    }

    void countBytes(u64 byteCount) {
        byteAccumulateOnThisTest += byteCount;
    }

    bool isTesting() {
        if (mode != TestMode::Testing) {
            return false;
        }

        u64 currentTime = core::getPerfCounter();

        if (openBlockCount > 0) {
            Assert(openBlockCount == closeBlockCount, "Unbalanced BeginTime/EndTime");
            Assert(byteAccumulateOnThisTest == targetProcessedByteCount, "Processed byte count missmatch");

            u64 elapsedTime = timeAccumulateOnThisTest;
            result.testCount += 1;
            result.totalTime += elapsedTime;
            if (result.maxTime < elapsedTime) {
                result.maxTime = elapsedTime;
            }

            if (result.minTime > elapsedTime) {
                result.minTime = elapsedTime;

                testsStartAt = currentTime;
            }

            openBlockCount = 0;
            closeBlockCount = 0;
            timeAccumulateOnThisTest = 0;
            byteAccumulateOnThisTest = 0;
        }

        if (currentTime - testsStartAt > tryForTime) {
            mode = TestMode::Completed;
            core::logDirectStd("                                                                                   \n");
            printTestWaveResults();
        }

        return true;
    }

    static void printTimeWithBandwith(const char* label, f64 CPUTime, u64 _CPUTimerFreq, u64 byteCount) {
        core::logDirectStd("%s: %.0f", label, CPUTime);
        if (_CPUTimerFreq > 0) {
            f64 seconds = CPUTime / f64(_CPUTimerFreq);
            core::logDirectStd(" (%fms)", 1000.0 * seconds);

            if (byteCount > 0) {
                f64 bestBandwidth = f64(byteCount) / (f64(core::CORE_GIGABYTE) * seconds);
                core::logDirectStd(" %fgb/s", bestBandwidth);
            }
        }
    }

    static void printTimeWithBandwith(const char* label, u64 CPUTime, u64 _CPUTimerFreq, u64 byteCount) {
        printTimeWithBandwith(label, f64(CPUTime), _CPUTimerFreq, byteCount);
    }

    void printTestWaveResults() {
        printTimeWithBandwith("Min", result.minTime, CPUTimerFreq, targetProcessedByteCount);
        core::logDirectStd("\n");

        printTimeWithBandwith("Max", result.maxTime, CPUTimerFreq, targetProcessedByteCount);
        core::logDirectStd("\n");

        if (result.testCount > 0) {
            printTimeWithBandwith("Avg", f64(result.totalTime) / f64(result.testCount), CPUTimerFreq, targetProcessedByteCount);
            core::logDirectStd("\n");
        }
    }
};

struct ReadParams {
    void* data;
    addr_size dataSize;
    core::StrBuilder<> fileName;
};

void testFRead(RepetitionTester& tester, ReadParams& params) {
    while(tester.isTesting()) {
        FILE* file = fopen(params.fileName.view().data(), "rb");
        Assert(file, "fread failed");

        tester.beginTime();
        auto result = fread(params.data, params.dataSize, 1, file);
        tester.endTime();

        Assert(result == 1, "fread failed");

        tester.countBytes(params.dataSize);

        fclose(file);
    }
}

#if OS_WIN == 1
  #include <io.h>        // _open, _read, _close
  #include <fcntl.h>     // _O_BINARY, _O_RDONLY
  #define  OPEN_FN  _open
  #define  READ_FN  _read
  #define  CLOSE_FN _close
  #define  PATH_CSTR(path)  (path)           // already narrow
#else
  #include <fcntl.h>     // open
  #include <unistd.h>    // read, close
  #define  OPEN_FN  open
  #define  READ_FN  read
  #define  CLOSE_FN close
  #define  PATH_CSTR(path)  (path)
#endif

void testRead(RepetitionTester& tester, ReadParams& params)
{
    while (tester.isTesting()) {

        /* open file unbuffered / binary */
        int fd = OPEN_FN(PATH_CSTR(params.fileName.view().data()),
#if OS_WIN == 1
                         _O_RDONLY | _O_BINARY);
#else
                         O_RDONLY);
#endif
        Assert(fd != -1, "open failed");

        /* time the raw read() / _read() */
        tester.beginTime();
        addr_size totalRead = 0;
        while (totalRead < params.dataSize) {
            addr_size chunk = params.dataSize - totalRead;
            auto n = READ_FN(fd,
                             static_cast<char*>(params.data) + totalRead,
#if OS_WIN == 1
                             static_cast<unsigned int>(chunk)
#else
                             chunk
#endif
            );
            Assert(n >= 0, "read failed");
            totalRead += static_cast<addr_size>(n);
            if (n == 0) break; /* EOF */
        }
        tester.endTime();

        Assert(totalRead == params.dataSize, "read size mismatch");

        tester.countBytes(params.dataSize);

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

    ReadParams readParams = {};
    {
        core::FileStat s;
        core::Expect(core::fileStat(cmdArgs.inFileSb.view().data(), s));
        Panic(s.size != 0, "Picked files should have size greater than 0.");

        readParams.data = reinterpret_cast<void*>(defaultAlloc<char>(s.size));
        readParams.dataSize = s.size;
        readParams.fileName = std::move(cmdArgs.inFileSb);
    }
    defer { defaultFree(readParams.data, readParams.dataSize); };

    struct TestCase {
        const char* fnName;
        void(*fn)(RepetitionTester& tester, ReadParams& params);
    };

    auto CPUTimerFreq = core::getCPUFrequencyHz();
    TestCase testedFunctions[2] = {
        { "fread", testFRead },
        { "read", testRead },
    };

    for (u32 i = 0; i < CORE_C_ARRLEN(testedFunctions); i++) {
        RepetitionTester tester = {};
        auto t = testedFunctions[i];

        core::logDirectStd("\n--- %s ---\n", t.fnName);
        tester.newTestWave(readParams.dataSize, CPUTimerFreq, 15);
        t.fn(tester, readParams);
    }

    return 0;
}

