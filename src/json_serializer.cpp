#include <json_serializer.h>

namespace core {

// Serializer:

bool JSONSerializer::obj() {
    json.append("{");
    return true;
}

bool JSONSerializer::objEnd(bool isLast) {
    json.append('}');
    if (!isLast) json.append(',');
    return true;
}

bool JSONSerializer::arr() {
    json.append("[");
    return true;
}

bool JSONSerializer::arrEnd(bool isLast) {
    json.append(']');
    if (!isLast) json.append(',');
    return true;
}

bool JSONSerializer::key(core::StrView& k) {
    json.append('"');
    json.append(k);
    json.append('"');
    json.append(':');
    return true;
}

bool serializeToJSON(JSONSerializer& s, i32 value) {
    char buf[core::maxDigitsBase10<i32>() + 1] = {};
    i32 n = snprintf(buf, core::maxDigitsBase10<i32>(), "%d", value);
    s.json.append(buf, n);
    return true;
}

bool serializeToJSON(JSONSerializer& s, u32 value) {
    char buf[core::maxDigitsBase10<u32>() + 1] = {};
    i32 n = snprintf(buf, core::maxDigitsBase10<u32>(), "%u", value);
    s.json.append(buf, n);
    return true;
}

bool serializeToJSON(JSONSerializer& s, f64 value) {
    char buf[64];
    i32 n = snprintf(buf, 64, "%.16f", value);
    s.json.append(buf, n);
    return true;
}

bool serializeToJSON(JSONSerializer& s, bool value) {
    if (value) s.json.append("true");
    else       s.json.append("false");
    return true;
}

bool serializeToJSON(JSONSerializer& s, core::StrView value) {
    s.json.append('"');
    s.json.append(value);
    s.json.append('"');
    return true;
}

// Deserializer:

void JSONDeserializer::resetReadIdx() {
    readIdx = 0;
}

void JSONDeserializer::reset(const char* cptr, addr_size len) {
    json = core::sv(cptr, len);
    resetReadIdx();
}

bool JSONDeserializer::obj() {
    detail::skipWhiteSpace(*this);
    if (json[readIdx] != '{') {
        return false;
    }
    ++readIdx;
    return true;
}

bool JSONDeserializer::objEnd(bool isLast) {
    detail::skipWhiteSpace(*this);
    if (json[readIdx] != '}') {
        return false;
    }
    ++readIdx;
    detail::skipWhiteSpace(*this);

    if(!isLast && !detail::readComma(*this)) {
        return false;
    }

    return true;
}

bool JSONDeserializer::arr() {
    detail::skipWhiteSpace(*this);
    if (json[readIdx] != '[') {
        return false;
    }
    ++readIdx;
    return true;
}

bool JSONDeserializer::arrEnd(bool isLast) {
    detail::skipWhiteSpace(*this);
    if (json[readIdx] != ']') {
        return false;
    }
    ++readIdx;
    detail::skipWhiteSpace(*this);

    if(!isLast && !detail::readComma(*this)) {
        return false;
    }

    return true;
}

bool JSONDeserializer::key(core::StrView& k) {
    detail::skipWhiteSpace(*this);
    if (json[readIdx] != '"') {
        return false;
    }

    ++readIdx;
    addr_size keyBegin = readIdx;

    while (json[readIdx] != '"') {
        ++readIdx;
        if (readIdx >= json.len()) {
            return false;
        }
    }

    k = core::sv(json.view().data() + keyBegin, readIdx - keyBegin);
    ++readIdx;

    detail::skipWhiteSpace(*this);
    if (json[readIdx] != ':') {
        k = core::sv();
        return false;
    }
    ++readIdx;

    return true;
}

bool deserializeFromJSON(JSONDeserializer& d, i32& value) {
    char* end;
    value = strtol(d.peek(), &end, 10);
    return true;
}

bool deserializeFromJSON(JSONDeserializer& d, u32& value) {
    char* end;
    value = strtoul(d.peek(), &end, 10);
    return true;
}

bool deserializeFromJSON(JSONDeserializer& d, f64& value) {
    char* end;
    value = strtod(d.peek(), &end);
    return true;
}

bool deserializeFromJSON(JSONDeserializer& d, bool& value) {
    const char* begin = d.peek();
    const char* end = d.peek() + d.json.len();

    if (!begin) {
        return false;
    }
    if (end - begin < 4) {
        return false;
    }

    if (core::cptrEq(begin, "true", 4)) {
        value = true;
        return true;
    }

    if (end - begin < 5) {
        return false;
    }

    if (core::cptrEq(begin, "false", 5)) {
        value = false;
        return true;
    }

    return false;
}

bool deserializeFromJSON(JSONDeserializer& d, core::StrView& value) {
    const char* begin = d.peek();
    const char* end = d.peek() + d.json.len();

    if (!begin) {
        return false;
    }
    if (*d.peek() != '"') {
        return false;
    }

    begin++; // skip the oppening "

    while (*begin != '"') {
        if (begin == end) {
            return false;
        }
        begin++;
    }

    value = core::sv(d.peek() + 1, begin - d.peek() - 1);
    return true;
}

namespace detail {

void skipWhiteSpace(JSONDeserializer& d) {
    while (d.readIdx < d.json.len() && core::isWhiteSpace(d.json[d.readIdx])) {
        d.readIdx++;
    }
}

bool readComma(JSONDeserializer& d) {
    skipWhiteSpace(d);
    if (d.json[d.readIdx] != ',') {
        return false;
    }
    d.readIdx++;
    return true;
}

void advanceToEndOfValue(JSONDeserializer& d) {
    const char* s = &d.json[d.readIdx];
    const char* end = &d.json.last();

    if (*s == '"') {
        s++;
        while (*s != '"') {
            ++s;
            if (s == end) {
                break;
            }
        }
        s++;
    }
    else {
        while (!core::isWhiteSpace(*s) && *s != ',' && *s != '}' && *s != ']') {
            ++s;
            if (s == end) {
                break;
            }
        }
    }

    d.readIdx = s - d.json.view().data();
}

} // namespace detail

} // namespace core
