//
// SpecificJSON.hpp
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


class SpecificException : public std::exception {
private:
    const char* reason;

public:
    SpecificException(const char* Reason) : reason(Reason) { }
    const char* what() const throw() { return reason; }
};


class ParserPool;


class SimpleValueParser {
protected:
    bool finished;
    const char* setFinished(const char* Endptr, ParserPool& Pool);
    const char* setFinished(const char* Endptr);

public:
    SimpleValueParser() : finished(true) { }
    virtual ~SimpleValueParser();

    bool Finished() const { return finished; }
    // Returns nullptr if not finished (reached end) or pointer where ended.
    virtual const char* Scan(
        const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false) = 0;

    const char* skipWhitespace(const char* Begin, const char* End);
    inline bool isWhitespace(const char C) {
        return C == ' ' || C == '\x9' || C == '\xA' || C == '\xD';
    }
};


class ParseFloat : public SimpleValueParser {
public:
    typedef float Type;
    enum Pool { Index = 0 }; // Has to match ParserPool order.

    const char* Scan(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);
};


class ParseString : public SimpleValueParser {
public:
    typedef std::string Type;

private:
    enum State { Normal, Escaped, Unicode };
    State state;
    int count;
    char hex_digits[4];

public:
    enum Pool { Index = 1 }; // Has to match ParserPool order.

    ParseString() : state(Normal), count(0) { }
    const char* Scan(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);
};


class ParseInt : public SimpleValueParser {
public:
    typedef int Type;
    enum Pool { Index = 2 }; // Has to match ParserPool order.

    const char* Scan(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);
};


// Holds lowest-level parsers and buffer they share.
class ParserPool {
public:
    ParserPool() { }
    ParserPool(const ParserPool&) = delete;
    ParserPool& operator=(const ParserPool&) = delete;
    virtual ~ParserPool();

    std::vector<char> buffer;

    enum Parsers { Float, String, Int }; // Match with order below.
    // Match order with parsers' Pool enum Index values.
    std::tuple<ParseFloat, ParseString, ParseInt> Parser;
    std::tuple<ParseFloat::Type, ParseString::Type, ParseInt::Type> Value;

    inline const char* skipWhitespace(const char* Begin, const char* End) {
        return std::get<0>(Parser).skipWhitespace(Begin, End);
    }
    inline bool isWhitespace(const char C) {
        return std::get<0>(Parser).isWhitespace(C);
    }
};


extern SpecificException NotFinished;

template<typename Parser>
class ParseArray : public SimpleValueParser {
public:
    typedef std::vector<typename Parser::Type> Type;

private:
    Type out;
    bool began, expect_number;

public:
    ParseArray() : began(false), expect_number(true) { }
    const char* Scan(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);

    void Swap(Type& Alt) {
        if (!Finished())
            throw NotFinished;
        std::swap(Alt, out);
        out.resize(0);
    }
};


extern SpecificException InvalidArrayStart;
extern SpecificException InvalidArraySeparator;

template<typename Parser>
const char* ParseArray<Parser>::Scan(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    Parser& p(std::get<Parser::Pool::Index>(Pool.Parser));
    typename Parser::Type& value(std::get<Parser::Pool::Index>(Pool.Value));
    if (!p.Finished()) {
        // In the middle of parsing value when buffer ended?
        Begin = p.Scan(Begin, End, Pool);
        if (Begin == nullptr)
            return setFinished(nullptr);
        out.push_back(value);
        expect_number = false;
    } else if (!began) {
        // Expect '[' on first call.
        if (*Begin != '[')
            throw InvalidArrayStart;
        began = expect_number = true;
        Begin = skipWhitespace(++Begin, End);
        if (Begin == nullptr)
            return setFinished(nullptr);
        if (*Begin == ']') {
            began = false; // In case caller re-uses. Out must be empty.
            return setFinished(++Begin);
        }
    } else if (out.empty()) {
        Begin = skipWhitespace(Begin, End);
        if (Begin == nullptr)
            return setFinished(nullptr);
        if (*Begin == ']') {
            began = false; // In case caller re-uses. Out must be empty.
            return setFinished(++Begin);
        }
    }
    while (Begin != End) {
        if (expect_number) {
            if (isWhitespace(*Begin)) {
                Begin = skipWhitespace(++Begin, End);
                if (Begin == nullptr)
                    return setFinished(nullptr);
            }
            // Now there should be the item to parse.
            Begin = p.Scan(Begin, End, Pool);
            if (Begin == nullptr)
                return setFinished(nullptr);
            out.push_back(value);
            expect_number = false;
        }
        // Comma, maybe surrounded by spaces.
        if (*Begin == ',') // Most likely unless prettified.
            ++Begin;
        else {
            Begin = skipWhitespace(Begin, End);
            if (Begin == nullptr)
                return setFinished(nullptr);
            if (*Begin == ']') {
                began = false;
                return setFinished(++Begin);
            }
            if (*Begin != ',')
                throw InvalidArraySeparator;
            Begin++;
        }
        expect_number = true;
    }
    return setFinished(nullptr);
}


template<typename Parser>
class ParseContainerArray : public SimpleValueParser {
public:
    typedef std::vector<typename Parser::Type> Type;

private:
    Parser p;
    Type out;
    bool began, expect_item;

public:
    ParseContainerArray() : began(false), expect_item(true) { }
    const char* Scan(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);

    void Swap(Type& Alt) {
        if (!Finished())
            throw NotFinished;
        std::swap(Alt, out);
        out.resize(0);
    }
};


template<typename Parser>
const char* ParseContainerArray<Parser>::Scan(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    if (!p.Finished()) {
        // In the middle of parsing value when buffer ended?
        Begin = p.Scan(Begin, End, Pool);
        if (Begin == nullptr)
            return setFinished(nullptr);
        out.push_back(typename Parser::Type());
        p.Swap(out.back());
        expect_item = false;
    } else if (!began) {
        // Expect '[' on first call.
        if (*Begin != '[')
            throw InvalidArrayStart;
        began = expect_item = true;
        Begin = skipWhitespace(++Begin, End);
        if (Begin == nullptr)
            return setFinished(nullptr);
        if (*Begin == ']') {
            began = false; // In case caller re-uses. Out must be empty.
            return setFinished(++Begin);
        }
    } else if (out.empty()) {
        Begin = skipWhitespace(Begin, End);
        if (Begin == nullptr)
            return setFinished(nullptr);
        if (*Begin == ']') {
            began = false; // In case caller re-uses. Out must be empty.
            return setFinished(++Begin);
        }
    }
    while (Begin != End) {
        if (expect_item) {
            if (isWhitespace(*Begin)) {
                Begin = skipWhitespace(++Begin, End);
                if (Begin == nullptr)
                    return setFinished(nullptr);
            }
            // Now there should be the item to parse.
            Begin = p.Scan(Begin, End, Pool);
            if (Begin == nullptr)
                return setFinished(nullptr);
            out.push_back(typename Parser::Type());
            p.Swap(out.back());
            expect_item = false;
        }
        // Comma, maybe surrounded by spaces.
        if (*Begin == ',') // Most likely unless prettified.
            ++Begin;
        else {
            Begin = skipWhitespace(Begin, End);
            if (Begin == nullptr)
                return setFinished(nullptr);
            if (*Begin == ']') {
                began = false;
                return setFinished(++Begin);
            }
            if (*Begin != ',')
                throw InvalidArraySeparator;
            Begin++;
        }
        expect_item = true;
    }
    return setFinished(nullptr);
}


class ValueStore;

// Helper class for implementation of KeyValues template.
class ScanningKeyValue {
protected:
    bool given;
    void Give(ValueStore* VS);

public:
    ScanningKeyValue() : given(false) { }
    virtual ~ScanningKeyValue();
    virtual SimpleValueParser& Scanner(ParserPool& Pool) = 0;
    virtual const char* Key() const = 0;
    virtual void Swap(ValueStore* VS, ParserPool& Pool) = 0;
    virtual bool Required() const = 0;
    bool Given() const { return given; }
};

// Helper class for implementation of KeyValues template.
template<const char* KeyString, typename Parser>
class KeyValue : public ScanningKeyValue {
public:
    typedef typename Parser::Type Type;
    const char* Key() const { return KeyString; }
    void Swap(ValueStore* VS, ParserPool& Pool);

    void Swap(Type& Alt, ParserPool& Pool) {
        std::swap(Alt, std::get<Parser::Pool::Index>(Pool.Value));
        given = false;
    }

    SimpleValueParser& Scanner(ParserPool& Pool) {
        given = true;
        return std::get<Parser::Pool::Index>(Pool.Parser);
    }

    bool Required() const { return false; }
};

// Helper class for implementation of KeyValues template.
template<const char* KeyString, typename Parser>
class RequiredKeyValue : public KeyValue<KeyString,Parser> {
public:
    bool Required() const { return true; }
};

// Helper class for implementation of KeyValues template.
template<const char* KeyString, typename Parser>
class KeyContainerValue : public ScanningKeyValue {
private:
    Parser p;

public:
    typedef typename Parser::Type Type;
    const char* Key() const { return KeyString; }
    void Swap(ValueStore* VS, ParserPool& Pool);

    void Swap(Type& Alt, ParserPool& Pool) {
        p.Swap(Alt);
        given = false;
    }

    SimpleValueParser& Scanner(ParserPool& Pool) {
        given = true;
        return p;
    }

    bool Required() const { return false; }
};

// Helper class for implementation of KeyValues template.
template<const char* KeyString, typename Parser>
class RequiredKeyContainerValue : public KeyContainerValue<KeyString,Parser> {
    bool Required() const { return true; }
};


// Helper class for implementation of ParseObject class.
template<class ... Fields>
class KeyValues {
private:
    std::vector<ScanningKeyValue*> ptrs;

    template<typename FuncObj, typename Tuple, size_t... IdxSeq>
    void apply_to_each(FuncObj&& fo, Tuple&& t, std::index_sequence<IdxSeq...>)
    {
        int a[] = {
            (fo(std::get<IdxSeq>(std::forward<Tuple>(t))), void(), 0) ...
        };
        (void)a; // Suppresses unused variable warning.
    }

    class Pusher {
    private:
        std::vector<ScanningKeyValue*>& tgt;
    public:
        Pusher(std::vector<ScanningKeyValue*>& Target) : tgt(Target) { }
        void operator()(ScanningKeyValue& SKV) { tgt.push_back(&SKV); }
    };

public:
    std::tuple<Fields...> fields;

    KeyValues() {
        Pusher psh(ptrs);
        apply_to_each(psh, fields,
            std::make_index_sequence<std::tuple_size<decltype(fields)>::value>
                {});
    }

    size_t size() const { return std::tuple_size<decltype(fields)>::value; }
    SimpleValueParser& Scanner(size_t Index, ParserPool& Pool) {
        return ptrs[Index]->Scanner(Pool);
    }
    ScanningKeyValue* KeyValue(size_t Index) { return ptrs[Index]; }
};

// Another derived class adds all the convenience methods that map to index.

class ValueStore {
protected:
    bool given;

private:
    void Give();
    friend class ScanningKeyValue;

public:
    ValueStore() : given(false) { }
    bool Given() const { return given; }
};

template<typename Parser>
class Value : public ValueStore {
public:
    typedef typename Parser::Type Type;
    Type value;
};

template<class ... Fields>
class NamelessValues {
private:
    std::vector<ValueStore*> ptrs;

    template<typename FuncObj, typename Tuple, size_t... IdxSeq>
    void apply_to_each(FuncObj&& fo, Tuple&& t, std::index_sequence<IdxSeq...>)
    {
        int a[] = {
            (fo(std::get<IdxSeq>(std::forward<Tuple>(t))), void(), 0) ...
        };
        (void)a; // Suppresses unused variable warning.
    }

    class Pusher {
    private:
        std::vector<ValueStore*>& tgt;
    public:
        Pusher(std::vector<ValueStore*>& Target) : tgt(Target) { }
        void operator()(ValueStore& VS) { tgt.push_back(&VS); }
    };

public:
    std::tuple<Fields...> fields;

    NamelessValues() {
        Pusher psh(ptrs);
        apply_to_each(psh, fields, std::make_index_sequence<std::tuple_size<decltype(fields)>::value> {});
    }

    size_t size() const { return std::tuple_size<decltype(fields)>::value; }
    ValueStore* operator[](size_t Index) { return ptrs[Index]; }
};

template<const char* KeyString, typename Parser>
void KeyValue<KeyString,Parser>::Swap(ValueStore* VS, ParserPool& Pool) {
    Value<Parser>* dst(static_cast<Value<Parser>*>(VS));
    std::swap(dst->value, std::get<Parser::Pool::Index>(Pool.Value));
    Give(VS);
}

template<const char* KeyString, typename Parser>
void KeyContainerValue<KeyString,Parser>::Swap(ValueStore* VS, ParserPool& Pool)
{
    Value<Parser>* dst(static_cast<Value<Parser>*>(VS));
    p.Swap(dst->value);
    Give(VS);
}

// The Values class used by the template should be derived from NamelessValues
// and it adds way to access the fields using sensibly named methods.

extern SpecificException InvalidKey;
extern SpecificException RequiredKeyNotGiven;

template<typename KeyValues, typename Values>
class ParseObject : public SimpleValueParser {
private:
    KeyValues parsers;
    Values out;
    int activating, active;
    enum State {
        NotStarted,
        PreKey,
        ExpectKey,
        PreColon,
        ExpectColon,
        PreValue,
        ExpectValue,
        PreComma,
        ExpectComma
    };
    State state;

    void setActivating(const std::string& Incoming) {
        for (size_t k = 0; k < parsers.size(); ++k)
            if (strcmp(Incoming.c_str(), parsers.KeyValue(k)->Key()) == 0) {
                activating = static_cast<int>(k);
                return;
            }
        throw InvalidKey;
    }

    const char* checkPassed(const char* Ptr) noexcept(false) {
        for (size_t k = 0; k < out.size(); ++k) {
            ScanningKeyValue* skv = parsers.KeyValue(k);
            ValueStore* vs = out[k];
            if (skv->Required() && !vs->Given())
                throw RequiredKeyNotGiven;
        }
        state = NotStarted;
        activating = -1;
        active = -1;
        return setFinished(Ptr);
    }

public:
    typedef decltype(out.fields) Type;

    ParseObject() : activating(-1), active(-1), state(NotStarted) { }

    const char* Scan(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);

    void Swap(Type& Alt) {
        std::swap(Alt, out.fields);
        out.fields = Type();
    }
};

extern SpecificException InvalidObjectStart;
extern SpecificException InvalidKeySeparator;
extern SpecificException InvalidValueSeparator;

template<typename KeyValues, typename Values>
const char* ParseObject<KeyValues,Values>::Scan(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    while (Begin != End) {
        // Re-order states to most expected first once works.
        switch (state) {
        case NotStarted:
            // Expect '{' on the first call.
            if (*Begin != '{')
                throw InvalidObjectStart;
            state = PreKey;
            ++Begin;
        case PreKey:
            if (isWhitespace(*Begin)) {
                Begin = skipWhitespace(++Begin, End);
                if (Begin == nullptr)
                    return setFinished(nullptr);
            }
            if (*Begin == '}')
                return checkPassed(++Begin);
            state = ExpectKey;
        case ExpectKey:
            Begin = std::get<ParserPool::String>(Pool.Parser).Scan(
                Begin, End, Pool);
            if (Begin == nullptr)
                return setFinished(nullptr);
            setActivating(std::get<ParserPool::String>(Pool.Value));
            state = PreColon;
        case PreColon:
            if (isWhitespace(*Begin)) {
                Begin = skipWhitespace(++Begin, End);
                if (Begin == nullptr)
                    return setFinished(nullptr);
            }
            state = ExpectColon;
        case ExpectColon:
            if (*Begin != ':')
                throw InvalidKeySeparator;
            state = PreValue;
            if (++Begin == End)
                return setFinished(nullptr);
        case PreValue:
            if (isWhitespace(*Begin)) {
                Begin = skipWhitespace(++Begin, End);
                if (Begin == nullptr)
                    return setFinished(nullptr);
            }
            active = activating;
            activating = -1;
            state = ExpectValue;
        case ExpectValue:
            Begin = parsers.Scanner(active, Pool).Scan(Begin, End, Pool);
            if (Begin == nullptr)
                return setFinished(nullptr);
            parsers.KeyValue(active)->Swap(out[active], Pool);
            active = -1;
            state = PreComma;
        case PreComma:
            if (isWhitespace(*Begin)) {
                Begin = skipWhitespace(++Begin, End);
                if (Begin == nullptr)
                    return setFinished(nullptr);
            }
            state = ExpectComma;
        case ExpectComma:
            if (*Begin == '}')
                return checkPassed(++Begin);
            if (*Begin != ',')
                throw InvalidValueSeparator;
            state = PreKey;
            ++Begin;
        }
    }
    return setFinished(nullptr);
}


extern SpecificException NumberNotFinite;

template<typename Sink>
void Write(Sink& S, double Value, std::vector<char>& Buffer) {
    if (isfinite(Value)) {
        int count = snprintf(&Buffer.front(), Buffer.size(),
            "%.*g", std::numeric_limits<double>::digits10, Value);
        if (count < static_cast<int>(Buffer.size()))
            S.write(&Buffer.front(), count);
        else {
            Buffer.resize(count + 1);
            Write(S, Value, Buffer);
        }
    } else
        throw NumberNotFinite;
}

template<typename Sink>
void Write(Sink& S, float Value, std::vector<char>& Buffer) {
    if (isfinite(Value)) {
        int count = snprintf(&Buffer.front(), Buffer.size(),
            "%.*g", std::numeric_limits<float>::digits10, double(Value));
        if (count < static_cast<int>(Buffer.size()))
            S.write(&Buffer.front(), count);
        else {
            Buffer.resize(count + 1);
            Write(S, Value, Buffer);
        }
    } else
        throw NumberNotFinite;
}

template<typename Sink>
void Write(Sink& S, int Value, std::vector<char>& Buffer) {
    int count = snprintf(&Buffer.front(), Buffer.size(), "%i", Value);
    if (count < static_cast<int>(Buffer.size()))
        S.write(&Buffer.front(), count);
    else {
        Buffer.resize(count + 1);
        Write(S, Value, Buffer);
    }
}

template<typename Sink>
void Write(
    Sink& S, const char* Begin, const char* End, std::vector<char>& Buffer)
{
    char quote = '"';
    char escaped[3] = "\\ ";
    S.write(&quote, 1);
    for (const char* curr = Begin; curr < End; ++curr) {
        escaped[1] = 0;
        switch (*curr) {
        case '"': escaped[1] = '"'; break;
        case '\\': escaped[1] = '\\'; break;
        case '\n': escaped[1] = 'n'; break;
        case '\t': escaped[1] = 't'; break;
        case '\r': escaped[1] = 'r'; break;
        case '\f': escaped[1] = 'f'; break;
        case '\b': escaped[1] = 'b'; break;
        }
        if (escaped[1]) {
            S.write(Begin, curr - Begin);
            S.write(escaped, 2);
            Begin = curr + 1;
        } else if (*curr < 0x20) {
            S.write(Begin, curr - Begin);
            char u[7] = "\\u00xx";
            char c = *curr & 0xf;
            u[4] = (c == *curr) ? '0' : '1';
            u[5] = (c < 10) ? '0' + c : 'a' + c - 10;
            S.write(u, 6);
            Begin = curr + 1;
        }
    }
    if (Begin < End)
        S.write(Begin, End - Begin);
    S.write(&quote, 1);
}

template<typename Sink>
void Write(Sink& S, const std::string& Value, std::vector<char>& Buffer) {
    Write(S, Value.c_str(), Value.c_str() + Value.size(), Buffer);
}

template<typename Sink, typename T>
void Write(Sink& S, const std::vector<T>& Value, std::vector<char>& Buffer);

template<typename Sink, typename T>
void Write(Sink& S, const T* Value, std::vector<char>& Buffer) {
    if (Value != nullptr)
        Write(S, *Value, Buffer);
    else {
        char null[] = "null";
        S.write(&null, 4);
    }
}

template<typename Sink>
void Write(Sink& S, const char* Value, std::vector<char>& Buffer) {
    if (Value != nullptr)
        Write(S, Value, Value + strlen(Value), Buffer);
    else {
        char null[] = "null";
        S.write(null, 4);
    }
}

template<typename Sink, typename ForwardIterator>
void Write(Sink& S,
    ForwardIterator& Begin, ForwardIterator& End, std::vector<char>& Buffer)
{
    char c = '[';
    S.write(&c, 1);
    if (Begin != End) {
        Write(S, *Begin, Buffer);
        c = ',';
        while (++Begin != End) {
            S.write(&c, 1);
            Write(S, *Begin, Buffer);
        }
    }
    c = ']';
    S.write(&c, 1);
}

template<typename Sink, typename T>
void Write(Sink& S, const std::vector<T>& Value, std::vector<char>& Buffer) {
    auto b = Value.cbegin();
    auto e = Value.cend();
    Write<Sink,typename std::vector<T>::const_iterator>(S, b, e, Buffer);
}

#endif
