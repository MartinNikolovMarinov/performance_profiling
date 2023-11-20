#include <init_core.h>

// Hashing global functions:

template<>
addr_size core::hash(const core::str_view& key) {
    addr_size h = addr_size(core::simple_hash_32(key.data(), key.len()));
    return h;
}

template<>
addr_size core::hash(const i32& key) {
    addr_size h = addr_size(core::simple_hash_32(reinterpret_cast<const void*>(&key), sizeof(key)));
    return h;
}

template<>
addr_size core::hash(const u32& key) {
    addr_size h = addr_size(core::simple_hash_32(reinterpret_cast<const void*>(&key), sizeof(key)));
    return h;
}

template<>
bool core::eq(const core::str_view& a, const core::str_view& b) {
    return a.eq(b);
}

template<>
bool core::eq(const i32& a, const i32& b) {
    return a == b;
}

template<>
bool core::eq(const u32& a, const u32& b) {
    return a == b;
}

// Allocator global functions:

void* std_allocator_static::alloc(addr_size size) noexcept {
    return g_stdAlloc.alloc(size);
}

void* std_allocator_static::calloc(addr_size count, addr_size size) noexcept {
    return g_stdAlloc.calloc(count, size);
}

void std_allocator_static::free(void* ptr) noexcept {
    g_stdAlloc.free(ptr);
}

addr_size std_allocator_static::used_mem() noexcept {
    return g_stdAlloc.used_mem();
}

const char* std_allocator_static::allocator_name() noexcept {
    return g_stdAlloc.allocator_name();
}

namespace {

void printUsage() {
    fmt::print(fmt::emphasis::bold, "Usage \n");
    fmt::print("----------------------------------------------\n");
    fmt::print("TODO: Describe this program briefly.\n");
    fmt::print("----------------------------------------------\n");
    fmt::print("option column               description column\n");
    fmt::print("----------------------------------------------\n");
    fmt::print("--help, -h                  print this message\n");
    fmt::print("--method, -m                method of generating random numbers. 0 means uniform and 1 means cluster\n");
    fmt::print("--random_seed, -r           seed for the random pairs\n");
    fmt::print("--pair_count, -p            number of pairs to generate\n");
}

};

core_context initCore(i32 argc, const char** argv) {
    core::set_global_assert_handler([](const char* failedExpr, const char* file, i32 line, const char* errMsg) {
        constexpr u32 stackFramesToSkip = 3;
        std::string trace = core::stacktrace(200, stackFramesToSkip);
        fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                   "[ASSERTION] [EXPR]: {}\n[FILE]: {}:{}\n[MSG]: {}\n",
                    failedExpr, file, line, errMsg);
        fmt::print(fmt::emphasis::bold, "[TRACE]:\n{}\n", trace);
        throw std::runtime_error("Assertion failed!");
    });

    core_context ret = {};

    core::flag_parser parser;
    parser.flag(&ret.cmdArgs.method, core::sv("method"), true, [](void* val) -> bool {
        i32* method = reinterpret_cast<i32*>(val);
        if (method != nullptr && (*method == 0 || *method == 1)) return true;
        fmt::print(stderr, fg(fmt::color::red) | fmt::emphasis::bold, "[ERROR]: Invalid argument for method.\n");
        return false;
    });
    parser.flag(&ret.cmdArgs.rndSeed, core::sv("rndSeed"), true);
    parser.flag(&ret.cmdArgs.pairCount, core::sv("pairCount"), true);
    parser.option(&ret.cmdArgs.printHelp, core::sv("help"), [] (bool val) {
        if (val) {
            printUsage();
            core::os_exit(EXIT_SUCCESS);
        }
    });

    parser.alias(core::sv("method"), core::sv("m"));
    parser.alias(core::sv("rndSeed"), core::sv("r"));
    parser.alias(core::sv("pairCount"), core::sv("p"));
    parser.alias(core::sv("help"), core::sv("h"));

    {
        auto res = parser.parse(argc, argv);
        if (res.has_err()) {
            fmt::print(stderr, fg(fmt::color::red) | fmt::emphasis::bold, "[ERROR]: Command line arguments parsing failed.\n");
            printUsage();
            core::os_exit(EXIT_FAILURE);
        }
    }

    return ret;
}
