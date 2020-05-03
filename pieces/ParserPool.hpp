class ParserPool {
public:
    ParserPool() { }
    ParserPool(const ParserPool&) = delete;
    ParserPool& operator=(const ParserPool&) = delete;
    virtual ~ParserPool();

    std::vector<char> buffer;

    enum Parsers { PoolIndexes };
    std::tuple<PoolParsers> Parser;
    std::tuple<PoolParserTypes> Value;

    inline const char* skipWhitespace(const char* Begin, const char* End) {
        return std::get<0>(Parser).skipWhitespace(Begin, End);
    }
    inline bool isWhitespace(const char C) {
        return std::get<0>(Parser).isWhitespace(C);
    }
};
