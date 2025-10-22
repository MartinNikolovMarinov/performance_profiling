#include <core_init.h>
#include <haversine_distance.h>
#include <repetition_tester.h>
#include <slow_json_decoder.h>
#include <pair.h>

struct CommandLineArguments {
    core::StrBuilder<> inFileSb;
    core::StrBuilder<> answersFileSb;
};

void printUsage() {
    core::logDirectStd("Usage: ./estimate_best_chunk_size_for_file_read [input.json]\n");
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

bool readAChunk(core::FileDesc& file, char* chunk, addr_size chunkSize, addr_size& outBytesRead) {
    auto res = core::fileRead(file, chunk, chunkSize);
    if (res.hasErr()) {
        outBytesRead = 0;
        return false; // hard error
    }

    outBytesRead = res.value();
    if (outBytesRead == 0) return false; // EOF
    return true;
}

const addr_size OS_PAGE_SIZE = core::getPageSize();

char* allocateAndTouch(addr_size size, addr_size& pageCount) {
    pageCount = (size + OS_PAGE_SIZE - 1) / OS_PAGE_SIZE; // round up
    char* pages = reinterpret_cast<char*>(core::Unpack(core::allocPages(pageCount)));
    core::memset(pages, '9', pageCount * OS_PAGE_SIZE);
    return pages;
}

void benchmarkAReadChunkSize(const char* fName, addr_size chunkSize, const char* tlabel) {
    addr_size pageCount = 0;
    char* chunk = allocateAndTouch(chunkSize, pageCount);
    defer { core::freePages(chunk, pageCount); };

    auto freq = core::getCPUFrequencyHz();
    auto wave = RepetitionTester::startTestWave(freq, 10, tlabel);

    core::FileStat s;
    core::fileStat(fName, s);
    addr_size fileSize = s.size;

    bool shouldStop = false;
    while (!shouldStop) {
        auto file = core::Unpack(core::fileOpen(fName, core::OpenMode::Read), "Failed to Open File");
        defer { core::fileClose(file); };

        core::memset(chunk, '\0', pageCount * OS_PAGE_SIZE);

        addr_size processed = 0;
        addr_size bytesRead = 0;

        auto singleTest = RepetitionTester::startSingleTest(wave, shouldStop);
        // Timed code:
        {
            while (readAChunk(file, chunk, chunkSize, bytesRead)) {
                processed += bytesRead;
            }
        }
        RepetitionTester::endSingleTest(wave, singleTest);
        RepetitionTester::setProcessedBytes(wave, processed);

        AssertFmt(processed == fileSize,
                  "File read mismatch: expected {} bytes, got {}", fileSize, processed);
    }

    RepetitionTester::endTestWave(wave);
}

i32 main(i32 argc, const char** argv) {
    coreInit();

    CommandLineArguments cmdArgs{};
    parserCmdArguments(argc, argv, cmdArgs);

    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 9,  "Test Chunk = 512 B");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 10, "Test Chunk = 1 KiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 11, "Test Chunk = 2 KiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 12, "Test Chunk = 4 KiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 13, "Test Chunk = 8 KiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 14, "Test Chunk = 16 KiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 15, "Test Chunk = 32 KiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 16, "Test Chunk = 64 KiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 17, "Test Chunk = 128 KiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 18, "Test Chunk = 256 KiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 19, "Test Chunk = 512 KiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 20, "Test Chunk = 1 MiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 21, "Test Chunk = 2 MiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 22, "Test Chunk = 4 MiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 23, "Test Chunk = 8 MiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 24, "Test Chunk = 16 MiB");
    benchmarkAReadChunkSize(cmdArgs.inFileSb.view().data(), 1 << 25, "Test Chunk = 32 MiB");

    return 0;
}
