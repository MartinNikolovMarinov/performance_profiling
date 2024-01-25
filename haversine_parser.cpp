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

struct JSONSerializer {
    void pushObject() {
        json.append('{');
    }

    void popObject(bool isLast = false) {
        json.append('}');
        if (!isLast) json.append(',');
    }

    void pushArray() {
        json.append('[');
    }

    void popArray(bool isLast = false) {
        json.append(']');
        if (!isLast) json.append(',');
    }

    void pushKey(core::StrView key) {
        json.append('"');
        json.append(key);
        json.append('"');
        json.append(':');
    }

    template <typename T>
    void pushValue(const T& value, bool isLast = false) {
        toJSON(*this, value);
        if (!isLast) json.append(',');
    }

    core::StrBuilder<> json;
};

struct JSONDeserializer {
    void resetReadIdx() {
        readIdx = 0;
    }

    void reset(const char* x, addr_size len) {
        json = core::sv(x, len);
        resetReadIdx();
    }

    bool readObjectBegin() {
        if (json[readIdx] != '{') {
            return false;
        }

        ++readIdx;
        return true;
    }

    bool readObjectEnd() {
        if (json[readIdx] != '}') {
            return false;
        }

        ++readIdx;
        return true;
    }

    bool readArrayBegin() {
        if (json[readIdx] != '[') {
            return false;
        }

        ++readIdx;
        return true;
    }

    bool readArrayEnd() {
        if (json[readIdx] != ']') {
            return false;
        }

        ++readIdx;
        return true;
    }

    bool readKey(core::StrView& key) {
        if (json[readIdx] != '"') {
            return false;
        }

        ++readIdx;
        addr_size keyBegin = readIdx;

        while (json[readIdx] != '"') {
            ++readIdx;
        }

        key = core::sv(json.view().data() + keyBegin, readIdx - keyBegin);
        ++readIdx;
        return true;
    }

    template <typename T>
    bool readValue(T& value) {
        if (!fromJSON(*this, value)) {
            return false;
        }

        if (json[readIdx] != ',') {
            return false;
        }

        ++readIdx;
        return true;
    }

    core::StrBuilder<> json;
    addr_size readIdx;
};

void toJSON(JSONSerializer& s, f64 value) {
    char buf[64];
    i32 n = snprintf(buf, 64, "%.16f", value);
    s.json.append(buf, n);
}

void toJSON(JSONSerializer& s, i32 value) {
    char buf[core::maxDigitsBase10<i32>() + 1] = {};
    i32 n = snprintf(buf, core::maxDigitsBase10<i32>(), "%d", value);
    s.json.append(buf, n);
}

void toJSON(JSONSerializer& s, u32 value) {
    char buf[core::maxDigitsBase10<u32>() + 1] = {};
    i32 n = snprintf(buf, core::maxDigitsBase10<u32>(), "%u", value);
    s.json.append(buf, n);
}

void toJSON(JSONSerializer& s, bool value) {
    if (value) {
        s.json.append("true");
    }
    else {
        s.json.append("false");
    }
}

void toJSON(JSONSerializer& s, core::StrView value) {
    s.json.append('"');
    s.json.append(value);
    s.json.append('"');
}

i32 main(i32 argc, const char** argv) {
    if (!initCore(argc, argv)) {
        printf("%s\n", "Failed to initialize core!");
        return -1;
    }

    JSONSerializer ser;
    ser.pushObject();
        ser.pushKey(core::sv("hello"));
        ser.pushValue(1);
        ser.pushObject();
            ser.pushKey(core::sv("world"));
            ser.pushValue(2.0f);
            ser.pushKey(core::sv("foo"));
            ser.pushValue(core::sv("bar"), true);
        ser.popObject();
        ser.pushKey(core::sv("bar"));
        ser.pushValue(true, true);
    ser.popObject(true);

    printf("%s\n", ser.json.view().data());

    // JSONDeserializer deser;
    // deser.json = core::move(ser.json);

    // CommandLineArguments cmdArgs;
    // if (!parserCmdArguments(argc, argv, cmdArgs)) {
    //     return -2;
    // }

    // printf("Input file: %s\n", cmdArgs.inFileSb.view().data());
    // printf("Expected answer: %.16f\n", cmdArgs.answer);

    return 0;
}

