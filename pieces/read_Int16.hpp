class ParseInt16 : public ValueParser {
public:
    typedef std::int16_t Type;
    enum Pool { Index = PoolIndex };

    const char* Parse(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);
};
