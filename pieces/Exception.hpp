// Except otherwise stated:
//
// Copyright © 2020 Ismo Kärkkäinen. All rights reserved.
//
// Licensed under Universal Permissive License. See License.txt.

#if !defined(SPECIFICJSON_HPP)
#define SPECIFICJSON_HPP

#include <exception>
#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <ctype.h>
#include <cstring>
#include <cstdio>
#if !defined(__GNUG__)
#include <cmath>
#else
#include <math.h>
#endif


namespace specjson {

class Exception : public std::exception {
private:
    const char* reason;

public:
    Exception(const char* Reason) : reason(Reason) { }
    const char* what() const throw() { return reason; }
};

class ParserPool;

class ValueParser {
protected:
    bool finished;
    const char* setFinished(const char* Endptr, ParserPool& Pool);
    const char* setFinished(const char* Endptr);

public:
    ValueParser() : finished(true) { }
    virtual ~ValueParser();

    bool Finished() const { return finished; }
    // Returns nullptr if not finished (reached end) or pointer where ended.
    virtual const char* Parse(
        const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false) = 0;

    const char* skipWhitespace(const char* Begin, const char* End);
    inline bool isWhitespace(const char C) {
        return C == ' ' || C == '\x9' || C == '\xA' || C == '\xD';
    }
};
