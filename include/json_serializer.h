#include <init_core.h>

namespace core {

struct JSONSerializer {
    core::StrBuilder<> json;

    bool obj();
    bool objEnd(bool isLast = false);

    bool arr();
    bool arrEnd(bool isLast = false);

    bool key(core::StrView& k);

    template <typename T>
    bool val(T& value, bool isLast = false) {
        if (!serializeToJSON(*this, value)) return false;
        if (!isLast) json.append(',');
        return true;
    }
};

bool serializeToJSON(JSONSerializer& s, i32 value);
bool serializeToJSON(JSONSerializer& s, u32 value);
bool serializeToJSON(JSONSerializer& s, f64 value);
bool serializeToJSON(JSONSerializer& s, bool value);
bool serializeToJSON(JSONSerializer& s, core::StrView value);

struct JSONDeserializer;

namespace detail {

void skipWhiteSpace(JSONDeserializer& d);
bool readComma(JSONDeserializer& d);
void advanceToEndOfValue(JSONDeserializer& d);

} // namespace detail

struct JSONDeserializer {
    core::StrBuilder<> json;
    addr_size readIdx;

    JSONDeserializer() : readIdx(0) {}

    void resetReadIdx();
    void reset(const char* cptr, addr_size len);

    inline const char* peek() {
        return json.view().data() + readIdx;
    }

    bool obj();
    bool objEnd(bool isLast = false);

    bool arr();
    bool arrEnd(bool isLast = false);

    bool key(core::StrView& k);

    template <typename T>
    bool val(T& value, bool isLast = false) {
        detail::skipWhiteSpace(*this);
        if (!deserializeFromJSON(*this, value)) {
            return false;
        }

        detail::advanceToEndOfValue(*this);

        if (!isLast && !detail::readComma(*this)) {
            return false;
        }

        return true;
    }
};

bool deserializeFromJSON(JSONDeserializer& d, i32& value);
bool deserializeFromJSON(JSONDeserializer& d, u32& value);
bool deserializeFromJSON(JSONDeserializer& d, f64& value);
bool deserializeFromJSON(JSONDeserializer& d, bool& value);
bool deserializeFromJSON(JSONDeserializer& d, core::StrView& value);

} // namespace core
