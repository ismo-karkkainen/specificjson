//
// unittest.cpp
//
// Copyright © 2020 Ismo Kärkkäinen. All rights reserved.
//
// Licensed under Universal Permissive License. See License.txt.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "specificjsontest.hpp"
#include <sstream>
#include <cstdint>
#include <cinttypes>

using namespace specjson;

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
        REQUIRE_THROWS_AS(Write(s, nan(""), buf), Exception);
    }
    SUBCASE("Inf") {
        std::basic_stringstream<char> s;
        REQUIRE_THROWS_AS(Write(s, 1.0f / 0.0f, buf), Exception);
    }
    SUBCASE("-Inf") {
        std::basic_stringstream<char> s;
        REQUIRE_THROWS_AS(Write(s, -1.0f / 0.0f, buf), Exception);
    }
}

static char value_buffer[] = "012345678901234567890123456789";
template<typename T>
static const char* as_string(const T Value) {
    if constexpr (0 < T(0) - T(1)) {
        snprintf(value_buffer, 31, "%" PRIuLEAST64, static_cast<std::uint_least64_t>(Value));
    } else {
        snprintf(value_buffer, 31, "%" PRIiLEAST64, static_cast<std::int_least64_t>(Value));
    }
    return value_buffer;
}

TEST_CASE("Write (u)int8") {
    std::vector<char> buf;
    SUBCASE("INT8_MIN") {
        std::basic_stringstream<char> s;
        std::int8_t v(INT8_MIN);
        Write(s, v, buf);
        REQUIRE(s.str() == as_string(std::int8_t(INT8_MIN)));
    }
    SUBCASE("INT8_MAX") {
        std::basic_stringstream<char> s;
        std::int8_t v(INT8_MAX);
        Write(s, v, buf);
        REQUIRE(s.str() == as_string(std::int8_t(INT8_MAX)));
    }
    SUBCASE("UINT8_MAX") {
        std::basic_stringstream<char> s;
        std::uint8_t v(UINT8_MAX);
        Write(s, v, buf);
        REQUIRE(s.str() == as_string(std::uint8_t(UINT8_MAX)));
    }
}

TEST_CASE("Write (u)int16") {
    std::vector<char> buf;
    SUBCASE("INT16_MIN") {
        std::basic_stringstream<char> s;
        std::int16_t v(INT16_MIN);
        Write(s, v, buf);
        REQUIRE(s.str() == as_string(std::int16_t(INT16_MIN)));
    }
    SUBCASE("INT16_MAX") {
        std::basic_stringstream<char> s;
        std::int16_t v(INT16_MAX);
        Write(s, v, buf);
        REQUIRE(s.str() == as_string(std::int16_t(INT16_MAX)));
    }
    SUBCASE("UINT16_MAX") {
        std::basic_stringstream<char> s;
        std::uint16_t v(UINT16_MAX);
        Write(s, v, buf);
        REQUIRE(s.str() == as_string(std::uint16_t(UINT16_MAX)));
    }
}

TEST_CASE("Write (u)int32") {
    std::vector<char> buf;
    SUBCASE("INT32_MIN") {
        std::basic_stringstream<char> s;
        std::int32_t v(INT32_MIN);
        Write(s, v, buf);
        REQUIRE(s.str() == as_string(std::int32_t(INT32_MIN)));
    }
    SUBCASE("INT32_MAX") {
        std::basic_stringstream<char> s;
        std::int32_t v(INT32_MAX);
        Write(s, v, buf);
        REQUIRE(s.str() == as_string(std::int32_t(INT32_MAX)));
    }
    SUBCASE("UINT32_MAX") {
        std::basic_stringstream<char> s;
        std::uint32_t v(UINT32_MAX);
        Write(s, v, buf);
        REQUIRE(s.str() == as_string(std::uint32_t(UINT32_MAX)));
    }
}

TEST_CASE("Write (u)int64") {
    std::vector<char> buf;
    SUBCASE("INT64_MIN") {
        std::basic_stringstream<char> s;
        std::int64_t v(INT64_MIN);
        Write(s, v, buf);
        REQUIRE(s.str() == as_string(std::int64_t(INT64_MIN)));
    }
    SUBCASE("INT64_MAX") {
        std::basic_stringstream<char> s;
        std::int64_t v(INT64_MAX);
        Write(s, v, buf);
        REQUIRE(s.str() == as_string(std::int64_t(INT64_MAX)));
    }
    SUBCASE("UINT64_MAX") {
        std::basic_stringstream<char> s;
        std::uint64_t v(UINT64_MAX);
        Write(s, v, buf);
        REQUIRE(s.str() == as_string(std::uint64_t(UINT64_MAX)));
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
        REQUIRE_THROWS_AS(Write(s, "a\x1 b", buf), Exception);
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
    SUBCASE("a[0x1f] b") {
        std::basic_stringstream<char> s;
        REQUIRE_THROWS_AS(Write(s, "a\x1f b", buf), Exception);
    }
    SUBCASE("a[0x20] b") {
        std::basic_stringstream<char> s;
        Write(s, "a\x20 b", buf);
        REQUIRE(s.str() == "\"a  b\"");
    }
    SUBCASE("a[0x0]") {
        std::basic_stringstream<char> s;
        char src[] = "a\x0";
        REQUIRE_THROWS_AS(Write(s, src, src + 2, buf), Exception);
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
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123.0f);
    }
    SUBCASE("456.789") {
        pp.buffer.resize(0);
        std::string s("456.789,");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 456.789f);
    }
    SUBCASE("1e6") {
        pp.buffer.resize(0);
        std::string s("1e6 ");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 1e6f);
    }
    SUBCASE("2E6") {
        pp.buffer.resize(0);
        std::string s("2E6 ");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 2e6f);
    }
    SUBCASE("-1.2") {
        pp.buffer.resize(0);
        std::string s("-1.2 ");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == -1.2f);
    }
    SUBCASE("+0.9") {
        pp.buffer.resize(0);
        std::string s("+0.9 ");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 0.9f);
    }
    SUBCASE("+|0.9") {
        pp.buffer.resize(0);
        std::string s0("+");
        std::string s("0.9 ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 0.9f);
    }
    SUBCASE("-|0.9") {
        pp.buffer.resize(0);
        std::string s0("-");
        std::string s("0.9 ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == -0.9f);
    }
    SUBCASE("|-0.9") {
        pp.buffer.resize(0);
        std::string s0("");
        std::string s("-0.9 ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == -0.9f);
    }
    SUBCASE("-0.9|") {
        pp.buffer.resize(0);
        std::string s0("-0.9");
        std::string s(" ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == -0.9f);
    }
    SUBCASE("12|.9e1") {
        pp.buffer.resize(0);
        std::string s0("12");
        std::string s(".9e1 ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 129.0f);
    }
    SUBCASE("1.9|e-2") {
        pp.buffer.resize(0);
        std::string s0("1.9");
        std::string s("e-2 ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 1.9e-2f);
    }
    SUBCASE("1.9|ee-2") {
        pp.buffer.resize(0);
        std::string s0("1.9");
        std::string s("ee-2 ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("1.9|eex2") {
        pp.buffer.resize(0);
        std::string s0("1.9");
        std::string s("eex2 ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("empty") {
        pp.buffer.resize(0);
        REQUIRE_THROWS_AS(parser.Parse(space, space + 1, pp), Exception);
    }
    SUBCASE("1e3e") {
        pp.buffer.resize(0);
        std::string s("1e3e");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 1e3f);
    }
    SUBCASE("0x1p3") {
        pp.buffer.resize(0);
        std::string s("0x1p3");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
}

TEST_CASE("String and escapes") {
    ParserPool pp;
    std::string& out(std::get<ParserPool::String>(pp.Value));
    ParseString& parser(std::get<ParserPool::String>(pp.Parser));
    SUBCASE("empty") {
        pp.buffer.resize(0);
        std::string s("\"\"");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "");
    }
    SUBCASE("string") {
        pp.buffer.resize(0);
        std::string s("\"string\"");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "string");
    }
    SUBCASE("str|ing") {
        pp.buffer.resize(0);
        std::string s0("\"str");
        std::string s("ing\"");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "string");
    }
    SUBCASE("a\\|\"b") {
        pp.buffer.resize(0);
        std::string s0("\"a\\");
        std::string s("\"b\"");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "a\"b");
    }
    SUBCASE("a\\\"b") {
        pp.buffer.resize(0);
        std::string s("\"a\\\"b\"");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "a\"b");
    }
    SUBCASE("a\\\"") {
        pp.buffer.resize(0);
        std::string s("\"a\\\"\"");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "a\"");
    }
    SUBCASE("\\\"b") {
        pp.buffer.resize(0);
        std::string s("\"\\\"b\"");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\"b");
    }
    SUBCASE("\\/\\\\\\b\\f\\n\\r\\t") {
        pp.buffer.resize(0);
        std::string s("\"\\/\\\\\\b\\f\\n\\r\\t\"");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "/\\\b\f\n\r\t");
    }
    SUBCASE("Invalid start") {
        pp.buffer.resize(0);
        std::string s("x");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("Invalid escape") {
        std::string valid("\"/\\bfnrtu");
        char escape[] = "\"\\ ";
        for (unsigned char u = 255; 31 < u; --u) {
            ParserPool pp2;
            ParseString& parser(std::get<ParserPool::String>(pp2.Parser));
            escape[2] = static_cast<char>(u);
            if (valid.find(escape[2]) == std::string::npos)
                REQUIRE_THROWS_AS(parser.Parse(escape, escape + 3, pp2), Exception);
        }
    }
    SUBCASE("Too small") {
        ParserPool pp2;
        ParseString& parser(std::get<ParserPool::String>(pp2.Parser));
        char c[] = "\" ";
        c[1] = 0x1f;
        REQUIRE_THROWS_AS(parser.Parse(c, c + 2, pp2), Exception);
    }
    SUBCASE("Too small") {
        ParserPool pp2;
        ParseString& parser(std::get<ParserPool::String>(pp2.Parser));
        char c[] = "\" ";
        c[1] = 0x1;
        REQUIRE_THROWS_AS(parser.Parse(c, c + 2, pp2), Exception);
    }
}

TEST_CASE("String Unicode") {
    ParserPool pp;
    std::string& out(std::get<ParserPool::String>(pp.Value));
    ParseString& parser(std::get<ParserPool::String>(pp.Parser));
    SUBCASE("\\u0079") {
        pp.buffer.resize(0);
        std::string s("\"\\u0079\"");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\x79");
    }
    SUBCASE("\\u0|079") {
        pp.buffer.resize(0);
        std::string s0("\"\\u0");
        std::string s("079\"");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\x79");
    }
    SUBCASE("\\u0080") {
        pp.buffer.resize(0);
        std::string s("\"\\u0080\"");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\xC2\x80");
    }
    SUBCASE("\\u07FF") {
        pp.buffer.resize(0);
        std::string s("\"\\u07FF\"");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\xDF\xBF");
    }
    SUBCASE("\\u07|FF") {
        pp.buffer.resize(0);
        std::string s0("\"\\u07");
        std::string s("FF\"");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\xDF\xBF");
    }
    SUBCASE("\\u0800") {
        pp.buffer.resize(0);
        std::string s("\"\\u0800\"");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(out == "\xE0\xA0\x80");
    }
    SUBCASE("\\uFFFF") {
        pp.buffer.resize(0);
        std::string s("\"\\uFFFF\"");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
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
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("-123") {
        pp.buffer.resize(0);
        std::string s("-123 ");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == -123);
    }
    SUBCASE("+123") {
        pp.buffer.resize(0);
        std::string s("+123 ");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("1|23") {
        pp.buffer.resize(0);
        std::string s0("1");
        std::string s("23 ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("1|2|3") {
        pp.buffer.resize(0);
        std::string s0("1");
        std::string s1("2");
        std::string s("3 ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s1.c_str(), s1.c_str() + s1.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("-|123") {
        pp.buffer.resize(0);
        std::string s0("-");
        std::string s("123 ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == -123);
    }
    SUBCASE("+|123") {
        pp.buffer.resize(0);
        std::string s0("+");
        std::string s("123 ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("|123") {
        pp.buffer.resize(0);
        std::string s0("");
        std::string s("123 ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("123|") {
        pp.buffer.resize(0);
        std::string s0("123");
        std::string s(" ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        REQUIRE(out == 123);
    }
    SUBCASE("123|x") {
        pp.buffer.resize(0);
        std::string s0("123");
        std::string s("x ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str());
        REQUIRE(out == 123);
    }
    SUBCASE("invalid") {
        pp.buffer.resize(0);
        std::string s("invalid");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("123|-456") {
        pp.buffer.resize(0);
        std::string s0("123");
        std::string s("-456 ");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
}

TEST_CASE("Float array") {
    ParserPool pp;
    ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type out;
    SUBCASE("[]") {
        pp.buffer.resize(0);
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s("[]");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.empty());
    }
    SUBCASE("[ ]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s("[ ]");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.empty());
    }
    SUBCASE("[\x9 1\xA\xD]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s("[ 1 ]");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 1);
        REQUIRE(out[0] == 1.0f);
    }
    SUBCASE("[ 1 ] again") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s("[ 1 ]");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(parser.Finished());
        s = std::string("[2]");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished());
        parser.Swap(out);
        REQUIRE(out.size() == 1);
        REQUIRE(out[0] == 2.0f);
    }
    SUBCASE("[1,2]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s("[1,2]");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0] == 1.0f);
        REQUIRE(out[1] == 2.0f);
    }
    SUBCASE("[|]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s0("[");
        std::string s("]");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.empty());
    }
    SUBCASE("[1,|2]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s0("[1,");
        std::string s("2]");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0] == 1.0f);
        REQUIRE(out[1] == 2.0f);
    }
    SUBCASE("[1|,2]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s0("[1");
        std::string s(",2]");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0] == 1.0f);
        REQUIRE(out[1] == 2.0f);
    }
    SUBCASE("[1.|0,2.0]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s0("[1.");
        std::string s("0,2.0]");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0] == 1.0f);
        REQUIRE(out[1] == 2.0f);
    }
    SUBCASE("[1,2|.0]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s0("[1,2");
        std::string s(".0]");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0] == 1.0f);
        REQUIRE(out[1] == 2.0f);
    }
    SUBCASE("[1,2|.0,3]") {
        out.resize(0);
        pp.buffer.resize(0);
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s0("[1,2");
        std::string s(".0,3]");
        REQUIRE(parser.Parse(s0.c_str(), s0.c_str() + s0.size(), pp) == nullptr);
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 3);
        REQUIRE(out[0] == 1.0f);
        REQUIRE(out[1] == 2.0f);
        REQUIRE(out[2] == 3.0f);
    }
}

TEST_CASE("Float array failures") {
    ParserPool pp;
    ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type out;
    SUBCASE("invalid") {
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s("invalid");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("]") {
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s("]");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("[,") {
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s("[,");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("[ ,") {
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s("[ ,");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("[1,,") {
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s("[1,,");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("[1,]") {
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s("[1,]");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("[1 , ]") {
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s("[1 , ]");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("[1") {
        ParseArray<std::vector<ParseFloat::Type>,ParseFloat> parser;
        std::string s("[1");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == nullptr);
        REQUIRE_THROWS_AS(parser.Swap(out), Exception);
    }
}

TEST_CASE("Float array array") {
    ParserPool pp;
    ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>>::Type out;
    SUBCASE("[[]]") {
        pp.buffer.resize(0);
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("[[]]");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 1);
        REQUIRE(out[0].empty());
    }
    SUBCASE("[[1]]") {
        pp.buffer.resize(0);
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("[[1]]");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 1);
        REQUIRE(out[0].size() == 1);
        REQUIRE(out[0][0] == 1.0f);
    }
    SUBCASE("[[1],[2]]") {
        pp.buffer.resize(0);
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("[[1],[2]]");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0].size() == 1);
        REQUIRE(out[1].size() == 1);
        REQUIRE(out[0][0] == 1.0f);
        REQUIRE(out[1][0] == 2.0f);
    }
    SUBCASE("[[1,2],[3,4]]") {
        pp.buffer.resize(0);
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("[[1,2],[3,4]]");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
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
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("[[1],[]]");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(out.size() == 2);
        REQUIRE(out[0].size() == 1);
        REQUIRE(out[1].empty());
        REQUIRE(out[0][0] == 1.0f);
    }
}

TEST_CASE("Array failures") {
    ParserPool pp;
    ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>>::Type out;
    SUBCASE("[[1][]]") {
        pp.buffer.resize(0);
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("[[1][]]");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("invalid") {
        pp.buffer.resize(0);
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("invalid");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("]") {
        pp.buffer.resize(0);
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("]");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("[,") {
        pp.buffer.resize(0);
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("[,");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("[ ,") {
        pp.buffer.resize(0);
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("[ ,");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("[[],,") {
        pp.buffer.resize(0);
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("[[],,");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("[[],]") {
        pp.buffer.resize(0);
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("[[],]");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("[[] , ]") {
        pp.buffer.resize(0);
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("[[] , ]");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("[[]") {
        pp.buffer.resize(0);
        ParseContainerArray<std::vector<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>::Type>,ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> parser;
        std::string s("[[]");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == nullptr);
        REQUIRE_THROWS_AS(parser.Swap(out), Exception);
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
        REQUIRE(kv.Scanner(pp).Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size() - 1);
        kv.Swap(out, pp);
        REQUIRE(out == 1.0f);
    }
}

TEST_CASE("Float array KeyValue") {
    ParserPool pp;
    SUBCASE("[1]") {
        KeyContainerValue<name2, ParseArray<std::vector<ParseFloat::Type>,ParseFloat>> kv;
        decltype(kv)::Type out;
        std::string s("[1]");
        REQUIRE(strcmp(kv.Key(), name2) == 0);
        REQUIRE(kv.Scanner(pp).Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
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
        KeyValues<KeyValue<name, ParseFloat>, KeyContainerValue<name2, ParseArray<std::vector<ParseFloat::Type>,ParseFloat>>> kvs;
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
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished() == true);
    }
    SUBCASE("Required field {}") {
        ParseObject<KeyValues<RequiredKeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{}");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("{\"name\":1}") {
        ParseObject<KeyValues<RequiredKeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{\"name\":1}");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished() == true);
        decltype(parser)::Type out;
        parser.Swap(out);
        REQUIRE(std::get<0>(out).value == 1.0f);
    }
    SUBCASE("{ \"name\" : 1 }") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{ \"name\" : 1 }");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished() == true);
        decltype(parser)::Type out;
        parser.Swap(out);
        REQUIRE(std::get<0>(out).value == 1.0f);
    }
    SUBCASE("{\"name\":1} no optional") {
        ParseObject<KeyValues<RequiredKeyValue<name, ParseFloat>,KeyValue<name2,ParseFloat>>,NamelessValues<Value<ParseFloat>,Value<ParseFloat>>> parser;
        std::string s("{\"name\":1}");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
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
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
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
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
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
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        REQUIRE(parser.Finished() == true);
        decltype(parser)::Type out;
        parser.Swap(out);
        s = std::string("{\"name2\":3,\"name\":4}");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
        parser.Swap(out);
        REQUIRE(parser.Finished() == true);
        REQUIRE(std::get<0>(out).Given() == true);
        REQUIRE(std::get<0>(out).value == 4.0f);
        REQUIRE(std::get<1>(out).Given() == true);
        REQUIRE(std::get<1>(out).value == 3.0f);
    }
    SUBCASE("{ \"name\" : [1] }") {
        ParseObject<KeyValues<KeyContainerValue<name, ParseArray<std::vector<ParseFloat::Type>,ParseFloat>>>,NamelessValues<Value<ParseArray<std::vector<ParseFloat::Type>,ParseFloat>>>> parser;
        std::string s("{ \"name\" : [1] }");
        REQUIRE(parser.Parse(s.c_str(), s.c_str() + s.size(), pp) == s.c_str() + s.size());
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
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("{:") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{:");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("{\"name\":,") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{\"name\":,");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("{\"invalid\"") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{\"invalid\"");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("{\"name\":1,,") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{\"name\":1,,");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("{\"name\":1:") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{\"name\":1:");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
    SUBCASE("{\"name\":\"invalid\"") {
        ParseObject<KeyValues<KeyValue<name, ParseFloat>>,NamelessValues<Value<ParseFloat>>> parser;
        std::string s("{\"name\":\"invalid\"");
        REQUIRE_THROWS_AS(parser.Parse(s.c_str(), s.c_str() + s.size(), pp), Exception);
    }
}
