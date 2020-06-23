extern const Exception NotFinished;
extern const Exception InvalidArrayStart;
extern const Exception InvalidArraySeparator;
extern const Exception SubContainerSizeVaries;

template<typename Container, typename Parser, bool Swaps = false>
class ParseArray : public ValueParser {
public:
    typedef Container Type;

private:
    Type out;
    bool began, expect_number;

public:
    ParseArray() : began(false), expect_number(true) { }
    const char* Parse(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);

    void Swap(Type& Alt) {
        if (!Finished())
            throw NotFinished;
        std::swap(Alt, out);
        out.resize(0);
    }
};

template<typename Container, typename Parser, bool Swaps>
const char* ParseArray<Container,Parser,Swaps>::Parse(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    const char* origin = Begin;
    Parser& p(std::get<Parser::Pool::Index>(Pool.Parser));
    typename Parser::Type& value(std::get<Parser::Pool::Index>(Pool.Value));
    if (!p.Finished()) {
        // In the middle of parsing value when buffer ended?
        Begin = p.Parse(Begin, End, Pool);
        if (Begin == nullptr)
            return setFinished(nullptr);
        if constexpr (Swaps) {
            out.push_back(typename Parser::Type());
            p.Swap(out.back());
        } else
            out.push_back(value);
        expect_number = false;
    } else if (!began) {
        // Expect '[' on first call.
        if (*Begin != '[')
            throw ContextException(InvalidArrayStart, origin, Begin, End);
        began = expect_number = true;
        Begin = skipWhitespace(++Begin, End);
        if (Begin == nullptr || !*Begin)
            return setFinished(nullptr);
        if (*Begin == ']') {
            began = false; // In case caller re-uses. Out must be empty.
            return setFinished(++Begin);
        }
    } else if (out.empty()) {
        Begin = skipWhitespace(Begin, End);
        if (Begin == nullptr || !*Begin)
            return setFinished(nullptr);
        if (*Begin == ']') {
            began = false; // In case caller re-uses. Out must be empty.
            return setFinished(++Begin);
        }
    }
    while (Begin != End) {
        if (expect_number) {
            Begin = skipWhitespace(Begin, End);
            if (Begin == nullptr || !*Begin)
                return setFinished(nullptr);
            // Now there should be the item to parse.
            Begin = p.Parse(Begin, End, Pool);
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
            if (Begin == nullptr || !*Begin)
                return setFinished(nullptr);
            if (*Begin == ']') {
                began = false;
                return setFinished(++Begin);
            }
            if (*Begin != ',')
                throw ContextException(InvalidArraySeparator, origin, Begin, End);
            Begin++;
        }
        expect_number = true;
    }
    return setFinished(nullptr);
}


template<typename Container, typename Parser, bool SameSize = false>
class ParseContainerArray : public ValueParser {
public:
    typedef Container Type;

private:
    Parser p;
    Type out;
    bool began, expect_item;

public:
    ParseContainerArray() : began(false), expect_item(true) { }
    const char* Parse(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);

    void Swap(Type& Alt) {
        if (!Finished())
            throw NotFinished;
        std::swap(Alt, out);
        out.resize(0);
    }
};


template<typename Container, typename Parser, bool SameSize>
const char* ParseContainerArray<Container,Parser,SameSize>::Parse(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    const char* origin = Begin;
    if (!p.Finished()) {
        // In the middle of parsing value when buffer ended?
        Begin = p.Parse(Begin, End, Pool);
        if (Begin == nullptr)
            return setFinished(nullptr);
        out.push_back(typename Parser::Type());
        p.Swap(out.back());
        if constexpr (SameSize) {
            if (out.front().size() != out.back().size())
                throw SubContainerSizeVaries;
        }
        expect_item = false;
    } else if (!began) {
        // Expect '[' on first call.
        if (*Begin != '[')
            throw ContextException(InvalidArrayStart, origin, Begin, End);
        began = expect_item = true;
        Begin = skipWhitespace(++Begin, End);
        if (Begin == nullptr || !*Begin)
            return setFinished(nullptr);
        if (*Begin == ']') {
            began = false; // In case caller re-uses. Out must be empty.
            return setFinished(++Begin);
        }
    } else if (out.empty()) {
        Begin = skipWhitespace(Begin, End);
        if (Begin == nullptr || !*Begin)
            return setFinished(nullptr);
        if (*Begin == ']') {
            began = false; // In case caller re-uses. Out must be empty.
            return setFinished(++Begin);
        }
    }
    while (Begin != End) {
        if (expect_item) {
            Begin = skipWhitespace(Begin, End);
            if (Begin == nullptr || !*Begin)
                return setFinished(nullptr);
            // Now there should be the item to parse.
            Begin = p.Parse(Begin, End, Pool);
            if (Begin == nullptr)
                return setFinished(nullptr);
            out.push_back(typename Parser::Type());
            p.Swap(out.back());
            if constexpr (SameSize) {
                if (out.front().size() != out.back().size())
                    throw SubContainerSizeVaries;
            }
            expect_item = false;
        }
        // Comma, maybe surrounded by spaces.
        if (*Begin == ',') // Most likely unless prettified.
            ++Begin;
        else {
            Begin = skipWhitespace(Begin, End);
            if (Begin == nullptr || !*Begin)
                return setFinished(nullptr);
            if (*Begin == ']') {
                began = false;
                return setFinished(++Begin);
            }
            if (*Begin != ',')
                throw ContextException(InvalidArraySeparator, origin, Begin, End);
            Begin++;
        }
        expect_item = true;
    }
    return setFinished(nullptr);
}
