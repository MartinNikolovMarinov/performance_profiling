#include <core_init.h>

#include <string.h>

struct RatSlot {
    const char* reg;
    i32 source;
};

RatSlot RAT_FILE[] = {
    { "rax", 0 },
    { "rbx", 1 },
    { "rcx", 2 },
    { "rdx", 3 }
};

i64 DATA[1024] = {};

i32 nextSlot = i32(CORE_C_ARRLEN(RAT_FILE));

RatSlot& getSlot(const char* reg) {
    for (i32 i = 0; i < i32(CORE_C_ARRLEN(RAT_FILE)); i++) {
        auto& slot = RAT_FILE[i];
        if (core::memcmp(reg, slot.reg, core::cstrLen(slot.reg)) == 0) {
            return slot;
        }
    }

    PanicFmt(false, "Register {} is not supported", reg);
    return RAT_FILE[0];
}

void printRegisterState(const char* command) {
    core::logDirectStd("After '{}':  \t", command);
    for (i32 i = 0; i < i32(CORE_C_ARRLEN(RAT_FILE)); i++) {
        auto& slot = RAT_FILE[i];
        core::logDirectStd("  {}({})={}n", slot.reg, slot.source, DATA[slot.source]);
    }
}

constexpr const char* readInst(const char** v) {
    const char*& p = *v;
    defer { p +=3; };

    if (core::memcmp(p, "mov", 3) == 0) {
        return "mov";
    }
    else if (core::memcmp(p, "inc", 3) == 0) {
        return "inc";
    }
    else if (core::memcmp(p, "dec", 3) == 0) {
        return "dec";
    }
    else if (core::memcmp(p, "sub", 3) == 0) {
        return "sub";
    }

    PanicFmt(false, "Instruction {} is not supported", core::sv(p, 4));
    return "cry";
}

constexpr const char* tryReadReg(const char** v) {
    const char*& p = *v;
    for (i32 i = 0; i < i32(CORE_C_ARRLEN(RAT_FILE)); i++) {
        auto& slot = RAT_FILE[i];
        if (core::memcmp(p, slot.reg, core::cstrLen(slot.reg)) == 0) {
            p += core::cstrLen(slot.reg);
            return slot.reg;
        }
    }

    return nullptr;
}

constexpr i64 readImm(const char** v) {
    const char*& p = *v;
    i64 ret = core::Unpack(core::cstrToInt<i64>(p, 1));
    p += core::digitCount(ret);
    return ret;
}

i32 main() {
    coreInit();

    // Chain dependency example:
    // const char* input =
    //     "mov rax, 1\n"
    //     "inc rax\n"
    //     "inc rax\n"
    //     "inc rax";

    // Broken chain dependency example:
    // const char* input =
    //     "mov rax, 1\n"
    //     "mov rcx, rax\n"
    //     "inc rcx\n"
    //     "mov rcx, rax\n"
    //     "inc rcx\n"
    //     "mov rcx, rax\n"
    //     "inc rcx\n"
    //     "mov rcx, rax\n"
    //     "inc rcx";

    // Homework input:
    const char* input =
        "mov rax, 1\n"
        "mov rbx, 1\n"
        "mov rcx, 1\n"
        "mov rdx, 1\n"
        "mov rax, rbx\n"
        "mov rcx, rdx\n"
        "mov rax, rcx\n"
        "mov rcx, rbx\n"
        "inc rax\n"
        "dec rcx\n"
        "sub rax, rbx\n"
        "sub rcx, rdx\n"
        "sub rax, rcx";

    const char* p = input;
    bool done = false;
    while (!done) {
        const char* curr = p;

        // This is supid:
        char* prev = strdup(curr);
        defer { std::free(prev); };
        i32 prevEnd = 0;

        // Skip to new line or break on EOF
        while (*p++ != '\n') {
            prevEnd++;
            if (*p == '\0') {
                done = true;
                break;
            }
        }

        prev[prevEnd] = '\0';

        // Parse the input

        const char* inst = nullptr, *reg1 = nullptr, *reg2 = nullptr;
        i64 immVal = 0;
        {
            constexpr bool __debugPrintParsed = false;

            inst = readInst(&curr);
            curr++; // skip ' '
            reg1 = tryReadReg(&curr);
            Assert(reg1);

            if (core::memcmp(inst, "inc", 3) == 0 ||
                core::memcmp(inst, "dec", 3) == 0
            ) {
                if constexpr (__debugPrintParsed) {
                    core::logDirectStd("inst = {}; reg1 = {}\n", inst, reg1);
                }
                Assert(*curr == '\n' || *curr == '\0');
            }
            else {
                curr += 2; // skip ', '
                reg2 = tryReadReg(&curr);
                immVal = 0;
                if (reg2 == nullptr) {
                    immVal = readImm(&curr);
                }
            }

            if constexpr (__debugPrintParsed) {
                core::logDirectStd("inst = {}; reg1 = {}; reg2 = {}; immVal = {}\n",
                                   inst, reg1, reg2, immVal);
            }
        }

        // Execute the commands

        if (core::memcmp(inst, "mov", 3) == 0) {
            auto& slot1 = getSlot(reg1);
            if (reg2) {
                // reg-to-reg write
                auto& slot2 = getSlot(reg2);
                slot1.source = slot2.source;
            }
            else {
                // write immediate
                slot1.source = nextSlot++;
                DATA[slot1.source] = immVal;
            }
        }
        else if (core::memcmp(inst, "inc", 3) == 0) {
            Assert(reg2 == nullptr && immVal == 0);
            auto& slot1 = getSlot(reg1);
            auto old = DATA[slot1.source];
            slot1.source = nextSlot++;
            DATA[slot1.source] = old + 1;
        }
        else if (core::memcmp(inst, "dec", 3) == 0) {
            Assert(reg2 == nullptr && immVal == 0);
            auto& slot1 = getSlot(reg1);
            auto old = DATA[slot1.source];
            slot1.source = nextSlot++;
            DATA[slot1.source] = old - 1;
        }
        else if (core::memcmp(inst, "sub", 3) == 0) {
            Assert(reg2);
            auto& slot1 = getSlot(reg1);
            auto& slot2 = getSlot(reg2);

            auto v1 = DATA[slot1.source];
            auto v2 = DATA[slot2.source];
            slot1.source = nextSlot++;
            DATA[slot1.source] = v1 - v2;
        }

        printRegisterState(prev);
        core::logDirectStd("\n");
    }
}
