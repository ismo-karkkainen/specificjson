class ParseUInt32 : public ValueParser {
public:
    typedef std::uint32_t Type;
    enum Pool { Index = PoolIndex };

    const char* Parse(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);
};
