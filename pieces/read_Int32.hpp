class ParseInt32 : public ValueParser {
public:
    typedef std::int32_t Type;
    enum Pool { Index = PoolIndex };

    const char* Parse(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);
};
