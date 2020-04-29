class ParseInt : public ValueParser {
public:
    typedef int Type;
    enum Pool { Index = PoolIndex };

    const char* Parse(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);
};
