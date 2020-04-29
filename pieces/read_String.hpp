class ParseString : public ValueParser {
public:
    typedef std::string Type;

private:
    enum State { Normal, Escaped, Unicode };
    State state;
    int count;
    char hex_digits[4];

public:
    enum Pool { Index = PoolIndex };

    ParseString() : state(Normal), count(0) { }
    const char* Parse(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);
};
