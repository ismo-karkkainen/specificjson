static Exception InvalidDouble("Invalid double.");

const char* ParseDouble::Parse(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    Type& out(std::get<ParseDouble::Pool::Index>(Pool.Value));
    char* end = nullptr;
    if (finished) {
        // Assumes LC_NUMERIC is "C" or close enough.
        out = strtod(Begin, &end);
        if (end != Begin && end != End) {
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
            throw InvalidDouble;
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
    out = strtod(&Pool.buffer.front(), &end);
    // Separator scan will throw if the string in source is not a number.
    // Require that all chars are the number as there was no separator copied.
    if (end != &Pool.buffer.back())
        throw InvalidDouble;
    return setFinished(Begin, Pool);
}
