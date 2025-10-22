#include <core_init.h>

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

    constexpr addr_off offset() {
        return static_cast<addr_off>(pos - start);
    }

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
        if (pos && *pos == ',') {
            pos++;
            pos = core::cstrSkipSpace(pos);
            return true;
        }
        return false;
    }

    constexpr bool skipPastSeparator(char sep) {
        pos = core::cstrSkipSpace(pos);
        if (pos && *pos == sep) {
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
