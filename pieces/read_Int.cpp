static Exception InvalidInt("Invalid integer.");

const char* ParseInt::Parse(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    Type& out(std::get<ParseInt::Pool::Index>(Pool.Value));
    char* end = nullptr;
    if (finished) {
        out = strtol(Begin, &end, 10);
        if (end == Begin) {
            if (Begin == End)
                return setFinished(nullptr, Pool);
            // Unless only sign fit, throw.
            if (Begin + 1 != End || (*Begin != '-' && *Begin != '+'))
                throw InvalidInt;
        } else if (end != End)
            return setFinished(end); // Good up to this.
        while (Begin != End)
            Pool.buffer.push_back(*Begin++);
        return setFinished(nullptr, Pool);
    }
    // Start of the number is in buffer. Input is null-terminated.
    while (('0' <= *Begin && *Begin <= '9') || *Begin == '-' || *Begin == '+')
        Pool.buffer.push_back(*Begin++);
    if (Begin == End) // Continues on and on?
        return setFinished(nullptr, Pool);
    Pool.buffer.push_back(0);
    out = strtol(&Pool.buffer.front(), &end, 10);
    // Separator scan will throw if the string in source is not a number.
    // Require that all chars are the number as there was no separator copied.
    if (end != &Pool.buffer.back())
        throw InvalidInt;
    return setFinished(Begin, Pool);
}
