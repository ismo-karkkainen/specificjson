class ParseUInt64 : public ValueParser {
public:
    typedef std::uint64_t Type;
    enum Pool { Index = PoolIndex };

    const char* Parse(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);
};
