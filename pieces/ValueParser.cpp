const char* ValueParser::setFinished(const char* Endptr, ParserPool& Pool) {
    finished = Endptr != nullptr;
    if (finished && !Pool.buffer.empty())
        Pool.buffer.resize(0);
    return Endptr;
}

const char* ValueParser::setFinished(const char* Endptr) {
    finished = Endptr != nullptr;
    return Endptr;
}

const char* ValueParser::skipWhitespace(const char* Begin, const char* End) {
    while (Begin != End && isWhitespace(*Begin))
        ++Begin;
    return (Begin != End) ? Begin : nullptr;
}

ValueParser::~ValueParser() { }
