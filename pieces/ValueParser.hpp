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
