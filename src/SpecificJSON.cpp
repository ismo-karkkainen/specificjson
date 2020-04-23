//
// SpecificJSON.cpp
//
// Copyright © 2020 Ismo Kärkkäinen. All rights reserved.
//
// Licensed under Universal Permissive License. See License.txt.

#include "SpecificJSON.hpp"
#include "testdoc.h"

namespace specjson {
// Exceptions thrown by templated code.
SpecificException NotFinished("Item not finished.");
SpecificException InvalidArrayStart("Expected '['");
SpecificException InvalidArraySeparator("Array, expected ','");
SpecificException InvalidObjectStart("Expected '{'");
SpecificException InvalidKeySeparator("Object, expected ','");
SpecificException InvalidKey("Object, unexpected key.");
SpecificException InvalidValueSeparator("Object, expected ':'");
SpecificException RequiredKeyNotGiven("Object, required key not given.");
SpecificException NumberNotFinite("Number not finite.");
}

using namespace specjson;

ParserPool::~ParserPool() { }

const char* SimpleValueParser::setFinished(const char* Endptr, ParserPool& Pool)
{
    finished = Endptr != nullptr;
    if (finished && !Pool.buffer.empty())
        Pool.buffer.resize(0);
    return Endptr;
}

const char* SimpleValueParser::setFinished(const char* Endptr) {
    finished = Endptr != nullptr;
    return Endptr;
}

const char* SimpleValueParser::skipWhitespace(const char* Begin, const char* End)
{
    while (Begin != End && isWhitespace(*Begin))
        ++Begin;
    return (Begin != End) ? Begin : nullptr;
}

SimpleValueParser::~SimpleValueParser() { }

static SpecificException InvalidFloat("Invalid float.");

const char* ParseFloat::Scan(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    Type& out(std::get<ParseFloat::Pool::Index>(Pool.Value));
    char* end = nullptr;
    if (finished) {
        // Assumes LC_NUMERIC is "C" or close enough for decimal point.
        out = strtof(Begin, &end);
        if (end != Begin && end != End) {
            // Detect hexadecimal significand and exponent, INF, NAN. ","
            // This block is about 6% of running time if you tolerate the above.
            while (Begin != end &&
                (('0' <= *Begin && *Begin <= '9') ||
                *Begin == '-' || *Begin == '+' || *Begin == '.' ||
                *Begin == 'e' || *Begin == 'E'))
                    ++Begin;
            if (Begin != end)
                throw InvalidFloat;
            return setFinished(end); // Good up to this.
        }
        // Copy good chars to buffer. Either end cut off or invalid.
        while (('0' <= *Begin && *Begin <= '9') || *Begin == '.' ||
            *Begin == 'e' || *Begin == 'E' ||
            *Begin == '-' || *Begin == '+')
                Pool.buffer.push_back(*Begin++);
        if (Begin != End) // Did not reach end of buffer, hence invalid.
            throw InvalidFloat;
        return setFinished(nullptr, Pool);
    }
    // Start of the number is in buffer. Input is null-terminated.
    while (('0' <= *Begin && *Begin <= '9') || *Begin == '.' ||
        *Begin == 'e' || *Begin == 'E' ||
        *Begin == '-' || *Begin == '+')
            Pool.buffer.push_back(*Begin++);
    if (Begin == End) // Continues on and on?
        return setFinished(nullptr, Pool);
    Pool.buffer.push_back(0);
    out = strtof(&Pool.buffer.front(), &end);
    // Separator scan will throw if the string in source is not a number.
    // Require that all chars are the number as there was no separator copied.
    if (end != &Pool.buffer.back())
        throw InvalidFloat;
    return setFinished(Begin, Pool);
}


static SpecificException StringStart("Expected '\"'.");
static SpecificException StringEscape("String with unknown escape.");
static SpecificException StringHexDigits("String with invalid hex digits.");
static SpecificException StringInvalidCharacter("String with invalid character.");

const char* ParseString::Scan(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    Type& out(std::get<Pool::Index>(Pool.Value));
    auto& buffer(Pool.buffer);
    if (finished) {
        if (*Begin != '"')
            throw StringStart;
        std::get<Pool::Index>(Pool.Value).resize(0);
        ++Begin;
    }
    while (Begin != End) {
        if (state == Normal) {
            while (Begin != End) {
                if (*Begin != '\\') {
                    if (*Begin != '"') {
                        if constexpr (static_cast<char>(0x80) < 0) {
                            // Signed char.
                            if (0 <= *Begin && *Begin < 32)
                                throw StringInvalidCharacter;
                        } else {
                            if (*Begin < 32)
                                throw StringInvalidCharacter;
                        }
                        buffer.push_back(*Begin++);
                    } else {
                        out.append(buffer.begin(), buffer.end());
                        return setFinished(Begin + 1, Pool);
                    }
                } else {
                    state = Escaped;
                    ++Begin;
                    break;
                }
            }
        } else if (state == Unicode) {
            hex_digits[count++] = *Begin++;
            if (count < 4)
                continue;
            count = 0;
            state = Normal;
            int value = 0;
            for (int k = 0; k < 4; ++k) {
                int m = 0;
                if ('0' <= hex_digits[k] && hex_digits[k] <= '9')
                    m = hex_digits[k] - '0';
                else if ('a' <= hex_digits[k] && hex_digits[k] <= 'f')
                    m = 10 + hex_digits[k] - 'a';
                else if ('A' <= hex_digits[k] && hex_digits[k] <= 'F')
                    m = 10 + hex_digits[k] - 'A';
                else
                    throw StringHexDigits;
                value = (value << 4) + m;
            }
            if (value < 0x80)
                buffer.push_back(static_cast<char>(value));
            else if (value < 0x800) {
                buffer.push_back(static_cast<char>(0xc0 | ((value >> 6) & 0x1f)));
                buffer.push_back(static_cast<char>(0x80 | (value & 0x3f)));
            } else {
                buffer.push_back(static_cast<char>(0xe0 | ((value >> 12) & 0xf)));
                buffer.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3f)));
                buffer.push_back(static_cast<char>(0x80 | (value & 0x3f)));
            }
        } else { // Escaped.
            state = Normal;
            switch (*Begin) {
            case '"':
            case '/':
            case '\\':
                buffer.push_back(*Begin);
                break;
            case 'b': buffer.push_back('\b'); break;
            case 'f': buffer.push_back('\f'); break;
            case 'n': buffer.push_back('\n'); break;
            case 'r': buffer.push_back('\r'); break;
            case 't': buffer.push_back('\t'); break;
            case 'u': state = Unicode; break;
            default:
                throw StringEscape;
            }
            ++Begin;
        }
    }
    return setFinished(nullptr, Pool);
}


static SpecificException InvalidInt("Invalid integer.");

const char* ParseInt::Scan(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    Type& out(std::get<ParseInt::Pool::Index>(Pool.Value));
    char* end = nullptr;
    if (finished) {
        out = strtol(Begin, &end, 10);
        if (end == Begin) {
            if (Begin == End)
                return setFinished(nullptr, Pool);
            // Unless only sign fit, throw.
            if (Begin + 1 != End || (*Begin != '-' && *Begin != '+'))
                throw InvalidInt;
        } else if (end != End)
            return setFinished(end); // Good up to this.
        while (Begin != End)
            Pool.buffer.push_back(*Begin++);
        return setFinished(nullptr, Pool);
    }
    // Start of the number is in buffer. Input is null-terminated.
    while (('0' <= *Begin && *Begin <= '9') || *Begin == '-' || *Begin == '+')
        Pool.buffer.push_back(*Begin++);
    if (Begin == End) // Continues on and on?
        return setFinished(nullptr, Pool);
    Pool.buffer.push_back(0);
    out = strtol(&Pool.buffer.front(), &end, 10);
    // Separator scan will throw if the string in source is not a number.
    // Require that all chars are the number as there was no separator copied.
    if (end != &Pool.buffer.back())
        throw InvalidInt;
    return setFinished(Begin, Pool);
}


ScanningKeyValue::~ScanningKeyValue() { }

void ScanningKeyValue::Give(ValueStore* VS) {
    given = false;
    VS->Give();
}

void ValueStore::Give() {
    given = true;
}



#if defined(TESTDOC_UNITTEST)
#include <sstream>
#include <cmath>


TEST_CASE("Write numbers") {
    std::vector<char> buf;
    SUBCASE("1.23456789f") {
        std::basic_stringstream<char> s;
        Write(s, 1.23456789f, buf);
        REQUIRE(s.str().size() == std::numeric_limits<float>::digits10 + 1);
        REQUIRE(s.str() == "1.23457"); // Rounded, hence 7 at end.
        REQUIRE(buf.size() > 0);
    }
    SUBCASE("1.234567890123456789") {
        std::basic_stringstream<char> s;
        Write(s, 1.234567890123456789, buf);
        REQUIRE(s.str().size() == std::numeric_limits<double>::digits10 + 1);
        REQUIRE(s.str() == "1.23456789012346"); // Rounded, hence 6 at end.
    }
    SUBCASE("12") {
        std::basic_stringstream<char> s;
        Write(s, 12, buf);
        REQUIRE(s.str() == "12");
    }
    SUBCASE("NaN") {
        std::basic_stringstream<char> s;
        REQUIRE_THROWS_AS(Write(s, nan(""), buf), SpecificException);
    }
    SUBCASE("Inf") {
        std::basic_stringstream<char> s;
        REQUIRE_THROWS_AS(Write(s, 1.0f / 0.0f, buf), SpecificException);
    }
    SUBCASE("-Inf") {
        std::basic_stringstream<char> s;
        REQUIRE_THROWS_AS(Write(s, -1.0f / 0.0f, buf), SpecificException);
    }
}

TEST_CASE("Write strings") {
    std::vector<char> buf;
    SUBCASE("normal") {
        std::basic_stringstream<char> s;
        Write(s, "normal", buf);
        REQUIRE(s.str() == "\"normal\"");
    }
    SUBCASE("std::string normal") {
        std::basic_stringstream<char> s;
        Write(s, std::string("normal"), buf);
        REQUIRE(s.str() == "\"normal\"");
    }
    SUBCASE("new\\nline") {
        std::basic_stringstream<char> s;
        Write(s, "new\nline", buf);
        REQUIRE(s.str() == "\"new\\nline\"");
    }
    SUBCASE("quo\"te") {
        std::basic_stringstream<char> s;
        Write(s, "quo\"te", buf);
        REQUIRE(s.str() == "\"quo\\\"te\"");
    }
    SUBCASE("back\\slash") {
        std::basic_stringstream<char> s;
        Write(s, "back\\slash", buf);
        REQUIRE(s.str() == "\"back\\\\slash\"");
    }
    SUBCASE("tab\\t") {
        std::basic_stringstream<char> s;
        Write(s, "tab\t", buf);
        REQUIRE(s.str() == "\"tab\\t\"");
    }
    SUBCASE("cr\\r") {
        std::basic_stringstream<char> s;
        Write(s, "cr\r", buf);
        REQUIRE(s.str() == "\"cr\\r\"");
    }
    SUBCASE("feed\\f") {
        std::basic_stringstream<char> s;
        Write(s, "feed\f", buf);
        REQUIRE(s.str() == "\"feed\\f\"");
    }
    SUBCASE("backspace\\b") {
        std::basic_stringstream<char> s;
        Write(s, "backspace\b", buf);
        REQUIRE(s.str() == "\"backspace\\b\"");
    }
}

TEST_CASE("Unicodes") {
    std::vector<char> buf;
    SUBCASE("a[0x1] b") {
        std::basic_stringstream<char> s;
        Write(s, "a\x1 b", buf);
        REQUIRE(s.str() == "\"a\\u0001 b\"");
    }
    SUBCASE("a[0x9] b") {
        std::basic_stringstream<char> s;
        Write(s, "a\x9 b", buf);
        REQUIRE(s.str() == "\"a\\t b\"");
    }
    SUBCASE("a[0xa] b") {
        std::basic_stringstream<char> s;
        Write(s, "a\xa b", buf);
        REQUIRE(s.str() == "\"a\\n b\"");
    }
    SUBCASE("a[0xf] b") {
        std::basic_stringstream<char> s;
        Write(s, "a\xf b", buf);
        REQUIRE(s.str() == "\"a\\u000f b\"");
    }
    SUBCASE("a[0x10] b") {
        std::basic_stringstream<char> s;
        Write(s, "a\x10 b", buf);
        REQUIRE(s.str() == "\"a\\u0010 b\"");
    }
    SUBCASE("a[0x1f] b") {
        std::basic_stringstream<char> s;
        Write(s, "a\x1f b", buf);
        REQUIRE(s.str() == "\"a\\u001f b\"");
    }
    SUBCASE("a[0x20] b") {
        std::basic_stringstream<char> s;
        Write(s, "a\x20 b", buf);
        REQUIRE(s.str() == "\"a  b\"");
    }
    SUBCASE("a[0x0] b") {
        std::basic_stringstream<char> s;
        char src[] = "a\x0 b";
        Write(s, src, src + 4, buf);
        REQUIRE(s.str() == "\"a\\u0000 b\"");
    }
}

TEST_CASE("Number vector") {
    std::vector<char> buf;
    SUBCASE("[]") {
        std::basic_stringstream<char> s;
        std::vector<int> src;
        Write(s, src, buf);
        REQUIRE(s.str() == "[]");
    }
    SUBCASE("[1]") {
        std::basic_stringstream<char> s;
        std::vector<int> src = { 1 };
        Write(s, src, buf);
        REQUIRE(s.str() == "[1]");
    }
    SUBCASE("[1,2]") {
        std::basic_stringstream<char> s;
        std::vector<int> src = { 1, 2 };
        Write(s, src, buf);
        REQUIRE(s.str() == "[1,2]");
    }
    SUBCASE("[1,2,3]") {
        std::basic_stringstream<char> s;
        std::vector<int> src = { 1, 2, 3 };
        Write(s, src, buf);
        REQUIRE(s.str() == "[1,2,3]");
    }
}

TEST_CASE("String vector") {
    std::vector<char> buf;
    SUBCASE("[]") {
        std::basic_stringstream<char> s;
        std::vector<std::string> src;
        Write(s, src, buf);
        REQUIRE(s.str() == "[]");
    }
    SUBCASE("[\"a\"]") {
        std::basic_stringstream<char> s;
        std::vector<std::string> src = { "a" };
        Write(s, src, buf);
        REQUIRE(s.str() == "[\"a\"]");
    }
    SUBCASE("[\"a\",\"b\"]") {
        std::basic_stringstream<char> s;
        std::vector<std::string> src = { "a", "b" };
        Write(s, src, buf);
        REQUIRE(s.str() == "[\"a\",\"b\"]");
    }
    SUBCASE("[\"a\",\"b\",\"c\"]") {
        std::basic_stringstream<char> s;
        std::vector<std::string> src = { "a", "b", "c" };
        Write(s, src, buf);
        REQUIRE(s.str() == "[\"a\",\"b\",\"c\"]");
    }
    SUBCASE("[\"a\"]") {
        std::basic_stringstream<char> s;
        std::vector<const char*> src = { "a" };
        Write(s, src, buf);
        REQUIRE(s.str() == "[\"a\"]");
    }
}

TEST_CASE("Number vector vector") {
    std::vector<char> buf;
    SUBCASE("[]") {
        std::basic_stringstream<char> s;
        std::vector<std::vector<int>> src;
        Write(s, src, buf);
        REQUIRE(s.str() == "[]");
    }
    SUBCASE("[[]]") {
        std::basic_stringstream<char> s;
        std::vector<std::vector<int>> src;
        src.push_back(std::vector<int>());
        Write(s, src, buf);
        REQUIRE(s.str() == "[[]]");
    }
    SUBCASE("[[1]]") {
        std::basic_stringstream<char> s;
        std::vector<std::vector<int>> src;
        src.push_back({ 1 });
        Write(s, src, buf);
        REQUIRE(s.str() == "[[1]]");
    }
    SUBCASE("[[1],[2]]") {
        std::basic_stringstream<char> s;
        std::vector<std::vector<int>> src;
        src.push_back({ 1 });
        src.push_back({ 2 });
        Write(s, src, buf);
        REQUIRE(s.str() == "[[1],[2]]");
    }
}

TEST_CASE("Floats") {
    ParserPool pp;
    float& out(std::get<ParserPool::Float>(pp.Value));
    ParseFloat& parser(std::get<ParserPool::Float>(pp.Parser));
    char space[] = " ";
    SUBCASE("123") {
        pp.buffer.resize(0);
        std::string s("123 ");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123.0f);
    }
    SUBCASE("456.789") {
        pp.buffer.resize(0);
        std::string s("456.789,");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 456.789f);
    }
    SUBCASE("1e6") {
        pp.buffer.resize(0);
        std::string s("1e6 ");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 1e6f);
    }
    SUBCASE("2E6") {
        pp.buffer.resize(0);
        std::string s("2E6 ");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 2e6f);
    }
    SUBCASE("-1.2") {
        pp.buffer.resize(0);
        std::string s("-1.2 ");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == -1.2f);
    }
    SUBCASE("+0.9") {
        pp.buffer.resize(0);
        std::string s("+0.9 ");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 0.9f);
    }
    SUBCASE("+|0.9") {
        pp.buffer.resize(0);
        std::string s0("+");
        std::string s("0.9 ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 0.9f);
    }
    SUBCASE("-|0.9") {
        pp.buffer.resize(0);
        std::string s0("-");
        std::string s("0.9 ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == -0.9f);
    }
    SUBCASE("|-0.9") {
        pp.buffer.resize(0);
        std::string s0("");
        std::string s("-0.9 ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == -0.9f);
    }
    SUBCASE("-0.9|") {
        pp.buffer.resize(0);
        std::string s0("-0.9");
        std::string s(" ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == -0.9f);
    }
    SUBCASE("12|.9e1") {
        pp.buffer.resize(0);
        std::string s0("12");
        std::string s(".9e1 ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 129.0f);
    }
    SUBCASE("1.9|e-2") {
        pp.buffer.resize(0);
        std::string s0("1.9");
        std::string s("e-2 ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 1.9e-2f);
    }
    SUBCASE("1.9|ee-2") {
        pp.buffer.resize(0);
        std::string s0("1.9");
        std::string s("ee-2 ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("1.9|eex2") {
        pp.buffer.resize(0);
        std::string s0("1.9");
        std::string s("eex2 ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("empty") {
        pp.buffer.resize(0);
        REQUIRE_THROWS_AS(parser.Scan(space, space + 1, pp), SpecificException);
    }
    SUBCASE("1e3e") {
        pp.buffer.resize(0);
        std::string s("1e3e");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 1e3f);
    }
    SUBCASE("0x1p3") {
        pp.buffer.resize(0);
        std::string s("0x1p3");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
}

TEST_CASE("String and escapes") {
    ParserPool pp;
    std::string& out(std::get<ParserPool::String>(pp.Value));
    ParseString& parser(std::get<ParserPool::String>(pp.Parser));
    SUBCASE("empty") {
        pp.buffer.resize(0);
        std::string s("\"\"");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "");
    }
    SUBCASE("string") {
        pp.buffer.resize(0);
        std::string s("\"string\"");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "string");
    }
    SUBCASE("str|ing") {
        pp.buffer.resize(0);
        std::string s0("\"str");
        std::string s("ing\"");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "string");
    }
    SUBCASE("a\\|\"b") {
        pp.buffer.resize(0);
        std::string s0("\"a\\");
        std::string s("\"b\"");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "a\"b");
    }
    SUBCASE("a\\\"b") {
        pp.buffer.resize(0);
        std::string s("\"a\\\"b\"");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "a\"b");
    }
    SUBCASE("a\\\"") {
        pp.buffer.resize(0);
        std::string s("\"a\\\"\"");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "a\"");
    }
    SUBCASE("\\\"b") {
        pp.buffer.resize(0);
        std::string s("\"\\\"b\"");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\"b");
    }
    SUBCASE("\\/\\\\\\b\\f\\n\\r\\t") {
        pp.buffer.resize(0);
        std::string s("\"\\/\\\\\\b\\f\\n\\r\\t\"");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "/\\\b\f\n\r\t");
    }
    SUBCASE("Invalid start") {
        pp.buffer.resize(0);
        std::string s("x");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("Invalid escape") {
        std::string valid("\"/\\bfnrtu");
        char escape[] = "\"\\ ";
        for (unsigned char u = 255; 31 < u; --u) {
            ParserPool pp2;
            ParseString& parser(std::get<ParserPool::String>(pp2.Parser));
            escape[2] = static_cast<char>(u);
            if (valid.find(escape[2]) == std::string::npos)
                REQUIRE_THROWS_AS(parser.Scan(escape, escape + 3, pp2), SpecificException);
        }
    }
    SUBCASE("Too small") {
        ParserPool pp2;
        ParseString& parser(std::get<ParserPool::String>(pp2.Parser));
        char c[] = "\" ";
        c[1] = 0x1f;
        REQUIRE_THROWS_AS(parser.Scan(c, c + 2, pp2), SpecificException);
    }
    SUBCASE("Too small") {
        ParserPool pp2;
        ParseString& parser(std::get<ParserPool::String>(pp2.Parser));
        char c[] = "\" ";
        c[1] = 0x1;
        REQUIRE_THROWS_AS(parser.Scan(c, c + 2, pp2), SpecificException);
    }
}

TEST_CASE("String Unicode") {
    ParserPool pp;
    std::string& out(std::get<ParserPool::String>(pp.Value));
    ParseString& parser(std::get<ParserPool::String>(pp.Parser));
    SUBCASE("\\u0079") {
        pp.buffer.resize(0);
        std::string s("\"\\u0079\"");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\x79");
    }
    SUBCASE("\\u0|079") {
        pp.buffer.resize(0);
        std::string s0("\"\\u0");
        std::string s("079\"");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\x79");
    }
    SUBCASE("\\u0080") {
        pp.buffer.resize(0);
        std::string s("\"\\u0080\"");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\xC2\x80");
    }
    SUBCASE("\\u07FF") {
        pp.buffer.resize(0);
        std::string s("\"\\u07FF\"");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\xDF\xBF");
    }
    SUBCASE("\\u07|FF") {
        pp.buffer.resize(0);
        std::string s0("\"\\u07");
        std::string s("FF\"");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\xDF\xBF");
    }
    SUBCASE("\\u0800") {
        pp.buffer.resize(0);
        std::string s("\"\\u0800\"");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\xE0\xA0\x80");
    }
    SUBCASE("\\uFFFF") {
        pp.buffer.resize(0);
        std::string s("\"\\uFFFF\"");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\xEF\xBF\xBF");
    }
}

TEST_CASE("Integer") {
    ParserPool pp;
    int& out(std::get<ParserPool::Int>(pp.Value));
    ParseInt& parser(std::get<ParserPool::Int>(pp.Parser));
    SUBCASE("123") {
        pp.buffer.resize(0);
        std::string s("123 ");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("-123") {
        pp.buffer.resize(0);
        std::string s("-123 ");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == -123);
    }
    SUBCASE("+123") {
        pp.buffer.resize(0);
        std::string s("+123 ");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("1|23") {
        pp.buffer.resize(0);
        std::string s0("1");
        std::string s("23 ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("1|2|3") {
        pp.buffer.resize(0);
        std::string s0("1");
        std::string s1("2");
        std::string s("3 ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s1.c_str(), s1.c_str() + s1.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("-|123") {
        pp.buffer.resize(0);
        std::string s0("-");
        std::string s("123 ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == -123);
    }
    SUBCASE("+|123") {
        pp.buffer.resize(0);
        std::string s0("+");
        std::string s("123 ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("|123") {
        pp.buffer.resize(0);
        std::string s0("");
        std::string s("123 ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("123|") {
        pp.buffer.resize(0);
        std::string s0("123");
        std::string s(" ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("123|x") {
        pp.buffer.resize(0);
        std::string s0("123");
        std::string s("x ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str());
        REQUIRE(out == 123);
    }
    SUBCASE("invalid") {
        pp.buffer.resize(0);
        std::string s("invalid");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("123|-456") {
        pp.buffer.resize(0);
        std::string s0("123");
        std::string s("-456 ");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
}

TEST_CASE("Float array") {
    ParserPool pp;
    ParseArray<ParseFloat>::Type out;
    SUBCASE("[]") {
        pp.buffer.resize(0);
        ParseArray<ParseFloat> parser;
        std::string s("[]");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.empty());
    }
    SUBCASE("[ ]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<ParseFloat> parser;
        std::string s("[ ]");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.empty());
    }
    SUBCASE("[\x9 1\xA\xD]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<ParseFloat> parser;
        std::string s("[ 1 ]");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 1);
        REQUIRE(out[0] == 1.0f);
    }
    SUBCASE("[ 1 ] again") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<ParseFloat> parser;
        std::string s("[ 1 ]");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(parser.Finished());
        s = std::string("[2]");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished());
        parser.Swap(out);
        REQUIRE(out.size() == 1);
        REQUIRE(out[0] == 2.0f);
    }
    SUBCASE("[1,2]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<ParseFloat> parser;
        std::string s("[1,2]");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0] == 1.0f);
        REQUIRE(out[1] == 2.0f);
    }
    SUBCASE("[|]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<ParseFloat> parser;
        std::string s0("[");
        std::string s("]");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.empty());
    }
    SUBCASE("[1,|2]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<ParseFloat> parser;
        std::string s0("[1,");
        std::string s("2]");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0] == 1.0f);
        REQUIRE(out[1] == 2.0f);
    }
    SUBCASE("[1|,2]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<ParseFloat> parser;
        std::string s0("[1");
        std::string s(",2]");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0] == 1.0f);
        REQUIRE(out[1] == 2.0f);
    }
    SUBCASE("[1.|0,2.0]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<ParseFloat> parser;
        std::string s0("[1.");
        std::string s("0,2.0]");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0] == 1.0f);
        REQUIRE(out[1] == 2.0f);
    }
    SUBCASE("[1,2|.0]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<ParseFloat> parser;
        std::string s0("[1,2");
        std::string s(".0]");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0] == 1.0f);
        REQUIRE(out[1] == 2.0f);
    }
    SUBCASE("[1,2|.0,3]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<ParseFloat> parser;
        std::string s0("[1,2");
        std::string s(".0,3]");
        REQUIRE(parser.Scan(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 3);
        REQUIRE(out[0] == 1.0f);
        REQUIRE(out[1] == 2.0f);
        REQUIRE(out[2] == 3.0f);
    }
}

TEST_CASE("Float array failures") {
    ParserPool pp;
    ParseArray<ParseFloat>::Type out;
    SUBCASE("invalid") {
        ParseArray<ParseFloat> parser;
        std::string s("invalid");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("]") {
        ParseArray<ParseFloat> parser;
        std::string s("]");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("[,") {
        ParseArray<ParseFloat> parser;
        std::string s("[,");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("[ ,") {
        ParseArray<ParseFloat> parser;
        std::string s("[ ,");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("[1,,") {
        ParseArray<ParseFloat> parser;
        std::string s("[1,,");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("[1,]") {
        ParseArray<ParseFloat> parser;
        std::string s("[1,]");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("[1 , ]") {
        ParseArray<ParseFloat> parser;
        std::string s("[1 , ]");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("[1") {
        ParseArray<ParseFloat> parser;
        std::string s("[1");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == nullptr);
        REQUIRE_THROWS_AS(parser.Swap(out), SpecificException);
    }
}

TEST_CASE("Float array array") {
    ParserPool pp;
    ParseContainerArray<ParseArray<ParseFloat>>::Type out;
    SUBCASE("[[]]") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("[[]]");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 1);
        REQUIRE(out[0].empty());
    }
    SUBCASE("[[1]]") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("[[1]]");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 1);
        REQUIRE(out[0].size() == 1);
        REQUIRE(out[0][0] == 1.0f);
    }
    SUBCASE("[[1],[2]]") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("[[1],[2]]");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0].size() == 1);
        REQUIRE(out[1].size() == 1);
        REQUIRE(out[0][0] == 1.0f);
        REQUIRE(out[1][0] == 2.0f);
    }
    SUBCASE("[[1,2],[3,4]]") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("[[1,2],[3,4]]");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0].size() == 2);
        REQUIRE(out[1].size() == 2);
        REQUIRE(out[0][0] == 1.0f);
        REQUIRE(out[0][1] == 2.0f);
        REQUIRE(out[1][0] == 3.0f);
        REQUIRE(out[1][1] == 4.0f);
    }
    SUBCASE("[[1],[]]") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("[[1],[]]");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0].size() == 1);
        REQUIRE(out[1].empty());
        REQUIRE(out[0][0] == 1.0f);
    }
}

TEST_CASE("Array failures") {
    ParserPool pp;
    ParseContainerArray<ParseArray<ParseFloat>>::Type out;
    SUBCASE("[[1][]]") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("[[1][]]");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("invalid") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("invalid");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("]") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("]");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("[,") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("[,");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("[ ,") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("[ ,");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("[[],,") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("[[],,");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("[[],]") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("[[],]");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("[[] , ]") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("[[] , ]");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("[[]") {
        pp.buffer.resize(0);
        ParseContainerArray<ParseArray<ParseFloat>> parser;
        std::string s("[[]");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == nullptr);
        REQUIRE_THROWS_AS(parser.Swap(out), SpecificException);
    }
}

static const char name[] = "name";
static const char name2[] = "name2";

TEST_CASE("Float KeyValue") {
    ParserPool pp;
    SUBCASE("1") {
        KeyValue<name, ParseFloat> kv;
        KeyValue<name, ParseFloat>::Type out;
        std::string s("1 ");
        REQUIRE(strcmp(kv.Key(), name) == 0);
        REQUIRE(kv.Scanner(pp).Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        kv.Swap(out, pp);
        REQUIRE(out == 1.0f);
    }
}

TEST_CASE("Float array KeyValue") {
    ParserPool pp;
    SUBCASE("[1]") {
        KeyContainerValue<name2, ParseArray<ParseFloat>> kv;
        decltype(kv)::Type out;
        std::string s("[1]");
        REQUIRE(strcmp(kv.Key(), name2) == 0);
        REQUIRE(kv.Scanner(pp).Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        kv.Swap(out, pp);
        REQUIRE(out.size() == 1);
        REQUIRE(out[0] == 1.0f);
    }
}

TEST_CASE("KeyValues") {
    ParserPool pp;
    SUBCASE("One member") {
        KeyValues<KeyValue<name, ParseFloat>> kvs;
        REQUIRE(kvs.size() == 1);
        REQUIRE(strcmp(std::get<0>(kvs.fields).Key(), name) == 0);
    }
    SUBCASE("Two members") {
        KeyValues<KeyValue<name, ParseFloat>, KeyContainerValue<name2, ParseArray<ParseFloat>>> kvs;
        REQUIRE(kvs.size() == 2);
        REQUIRE(strcmp(std::get<0>(kvs.fields).Key(), name) == 0);
        REQUIRE(strcmp(std::get<1>(kvs.fields).Key(), name2) == 0);
    }
    SUBCASE("One required member") {
        KeyValues<RequiredKeyValue<name, ParseFloat>> kvs;
        REQUIRE(kvs.size() == 1);
        REQUIRE(strcmp(std::get<0>(kvs.fields).Key(), name) == 0);
    }
}

TEST_CASE("KeyValue to Value swap") {
    ParserPool pp;
    SUBCASE("Float swap") {
        KeyValues<KeyValue<name, ParseFloat>> kvs;
        std::get<ParserPool::Float>(pp.Value) = 1.0f;
        NamelessValues<Value<ParseFloat>> vs;
        std::get<0>(kvs.fields).Swap(reinterpret_cast<ValueStore*>(&std::get<0>(vs.fields)), pp);
        REQUIRE(std::get<0>(vs.fields).value == 1.0f);
    }
    SUBCASE("Required float swap") {
        KeyValues<RequiredKeyValue<name, ParseFloat>> kvs;
        std::get<ParserPool::Float>(pp.Value) = 1.0f;
        NamelessValues<Value<ParseFloat>> vs;
        std::get<0>(kvs.fields).Swap(reinterpret_cast<ValueStore*>(&std::get<0>(vs.fields)), pp);
        REQUIRE(std::get<0>(vs.fields).value == 1.0f);
    }
}

TEST_CASE("Object") {
    ParserPool pp;
    SUBCASE("{}") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{}");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished() == true);
    }
    SUBCASE("Required field {}") {
        ParseObject<KeyValues<RequiredKeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{}");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("{\"name\":1}") {
        ParseObject<KeyValues<RequiredKeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{\"name\":1}");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished() == true);
        decltype(parser)::Type out;
        parser.Swap(out);
        REQUIRE(std::get<0>(out).value == 1.0f);
    }
    SUBCASE("{ \"name\" : 1 }") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{ \"name\" : 1 }");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished() == true);
        decltype(parser)::Type out;
        parser.Swap(out);
        REQUIRE(std::get<0>(out).value == 1.0f);
    }
    SUBCASE("{\"name\":1} no optional") {
        ParseObject<KeyValues<RequiredKeyValue<name, ParseFloat>,KeyValue<name2,ParseFloat>>,NamelessValues<Value<ParseFloat>,Value<ParseFloat>>> parser;
        std::string s("{\"name\":1}");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished() == true);
        decltype(parser)::Type out;
        parser.Swap(out);
        REQUIRE(std::get<0>(out).Given() == true);
        REQUIRE(std::get<0>(out).value == 1.0f);
        REQUIRE(std::get<1>(out).Given() == false);
    }
    SUBCASE("{\"name\":1,\"name2\":2}") {
        ParseObject<KeyValues<RequiredKeyValue<name, ParseFloat>,KeyValue<name2,ParseFloat>>,NamelessValues<Value<ParseFloat>,Value<ParseFloat>>> parser;
        std::string s("{\"name\":1,\"name2\":2}");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished() == true);
        decltype(parser)::Type out;
        parser.Swap(out);
        REQUIRE(std::get<0>(out).Given() == true);
        REQUIRE(std::get<0>(out).value == 1.0f);
        REQUIRE(std::get<1>(out).Given() == true);
        REQUIRE(std::get<1>(out).value == 2.0f);
    }
    SUBCASE("{\"name2\":2,\"name\":1}") {
        ParseObject<KeyValues<RequiredKeyValue<name, ParseFloat>,KeyValue<name2,ParseFloat>>,NamelessValues<Value<ParseFloat>,Value<ParseFloat>>> parser;
        std::string s("{\"name2\":2,\"name\":1}");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished() == true);
        decltype(parser)::Type out;
        parser.Swap(out);
        REQUIRE(std::get<0>(out).Given() == true);
        REQUIRE(std::get<0>(out).value == 1.0f);
        REQUIRE(std::get<1>(out).Given() == true);
        REQUIRE(std::get<1>(out).value == 2.0f);
    }
    SUBCASE("{\"name2\":2,\"name\":1} and again") {
        ParseObject<KeyValues<RequiredKeyValue<name, ParseFloat>,KeyValue<name2,ParseFloat>>,NamelessValues<Value<ParseFloat>,Value<ParseFloat>>> parser;
        std::string s("{\"name2\":2,\"name\":1}");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished() == true);
        decltype(parser)::Type out;
        parser.Swap(out);
        s = std::string("{\"name2\":3,\"name\":4}");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(parser.Finished() == true);
        REQUIRE(std::get<0>(out).Given() == true);
        REQUIRE(std::get<0>(out).value == 4.0f);
        REQUIRE(std::get<1>(out).Given() == true);
        REQUIRE(std::get<1>(out).value == 3.0f);
    }
    SUBCASE("{ \"name\" : [1] }") {
        ParseObject<KeyValues<KeyContainerValue<name, ParseArray<ParseFloat>>>,NamelessValues<Value<ParseArray<ParseFloat>>>> parser;
        std::string s("{ \"name\" : [1] }");
        REQUIRE(parser.Scan(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished() == true);
        decltype(parser)::Type out;
        parser.Swap(out);
        REQUIRE(std::get<0>(out).value.size() == 1);
        REQUIRE(std::get<0>(out).value[0] == 1.0f);
    }
}

TEST_CASE("Object invalid") {
    ParserPool pp;
    SUBCASE("invalid") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("invalid");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("{:") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{:");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("{\"name\":,") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{\"name\":,");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("{\"invalid\"") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{\"invalid\"");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("{\"name\":1,,") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{\"name\":1,,");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("{\"name\":1:") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{\"name\":1:");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
    SUBCASE("{\"name\":\"invalid\"") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{\"name\":\"invalid\"");
        REQUIRE_THROWS_AS(parser.Scan(s.c_str(), s.c_str() + s.size(), pp), SpecificException);
    }
}

#endif
