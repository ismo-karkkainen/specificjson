static const Exception InvalidFloat("Invalid float.");

const char* specjson::ParseFloat::Parse(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    Type& out(std::get<ParseFloat::Pool::Index>(Pool.Value));
    char* end = nullptr;
    if (finished) {
        // Assumes LC_NUMERIC is "C" or close enough.
        out = strtof(Begin, &end);
        // It is possible the exponent is in next buffer.
        if (end != Begin && end != End && !(*end == 'e' || *end == 'E')) {
            // Detect hexadecimal significand and exponent, INF, NAN. ","
            // This block is about 6% of parsing time if you tolerate the above.
            while (Begin != end &&
                (('0' <= *Begin && *Begin <= '9') ||
                *Begin == '-' || *Begin == '+' || *Begin == '.' ||
                *Begin == 'e' || *Begin == 'E'))
                    ++Begin;
            if (Begin != end)
                throw InvalidFloat;
            return setFinished(end); // Good up to this.
        }
        // Copy good chars to buffer. Either end cut off or invalid.
        while (('0' <= *Begin && *Begin <= '9') || *Begin == '.' ||
            *Begin == 'e' || *Begin == 'E' ||
            *Begin == '-' || *Begin == '+')
                Pool.buffer.push_back(*Begin++);
        if (Begin != End) // Did not reach end of buffer, hence invalid.
            throw InvalidFloat;
        return setFinished(nullptr, Pool);
    }
    // Start of the number is in buffer. Input is null-terminated.
    while (('0' <= *Begin && *Begin <= '9') || *Begin == '.' ||
        *Begin == 'e' || *Begin == 'E' ||
        *Begin == '-' || *Begin == '+')
            Pool.buffer.push_back(*Begin++);
    if (Begin == End) // Continues on and on?
        return setFinished(nullptr, Pool);
    Pool.buffer.push_back(0);
    out = strtof(&Pool.buffer.front(), &end);
    // Separator scan will throw if the string in source is not a number.
    // Require that all chars are the number as there was no separator copied.
    if (end != &Pool.buffer.back())
        throw InvalidFloat;
    return setFinished(Begin, Pool);
}
