#include <core_init.h>
#include <os_metrics.h>
#include <utils.h>

PRAGMA_WARNING_SUPPRESS_ALL

struct CommandLineArguments {
    i32 pageCount = -1;
};

void printUsage() {
    core::logDirectStd("Usage: ./page_fault_probing [page count]\n");
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
                cmdArgs.pageCount = core::Unpack(core::cstrToInt<i32>(arg.data(), arg.len()));
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

    addr_size pageSize = core::getPageSize();
    i32 pageCount = cmdArgs.pageCount;
    addr_size totalSize = pageSize * addr_size(pageCount);

    core::logDirectStd("Page Count, Touch Count, Fault Count, Extra Faults\n");

    for (i32 touchCount = 0; touchCount <= pageCount; touchCount++) {
        addr_size touchSize = pageSize * addr_size(touchCount);
        u8* data = reinterpret_cast<u8*>(core::Unpack(core::allocPages(totalSize)));

        u64 startFaultCount = readOSPageFaultCount();

        // Read front-to-back
        for (u64 i = 0; i < touchSize; i++) {
            data[i] = u8(i);
        }

        // // Read back-to-front
        // for (u64 i = touchSize == 0 ? 0 : touchSize - 1; i > 0; i--) {
        //     data[i] = u8(i);
        // }

        u64 endFaultCount = readOSPageFaultCount();
        u64 faultCount = endFaultCount - startFaultCount;

        core::logDirectStd("%llu, %llu, %llu, %llu\n", pageCount, touchCount, faultCount, faultCount - touchCount);

        core::freePages(data, totalSize);
    }

    return 0;
}
