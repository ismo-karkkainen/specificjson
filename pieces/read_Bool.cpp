static const Exception InvalidBool("Invalid Boolean.");

const char* specjson::ParseBool::Parse(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    Type& out(std::get<ParseBool::Pool::Index>(Pool.Value));
    if (!finished) {
        out = Pool.buffer.front() == 't';
    } else {
        if (!*Begin)
            return setFinished(nullptr, Pool);
        else if (*Begin == 't') {
            out = true; // Possibly.
        } else if (*Begin == 'f') {
            out = false; // Possibly.
        } else
            throw InvalidBool;
        Pool.buffer.push_back(*Begin++);
    }
    if (Pool.buffer.size() == 1) {
        if (Begin == End)
            return setFinished(nullptr, Pool);
        if (*Begin != (out ? 'r' : 'a'))
            throw InvalidBool;
        Pool.buffer.push_back(*Begin++);
    }
    if (Pool.buffer.size() == 2) {
        if (Begin == End)
            return setFinished(nullptr, Pool);
        if (*Begin != (out ? 'u' : 'l'))
            throw InvalidBool;
        Pool.buffer.push_back(*Begin++);
    }
    if (Pool.buffer.size() == 3) {
        if (Begin == End)
            return setFinished(nullptr, Pool);
        if (*Begin != (out ? 'e' : 's'))
            throw InvalidBool;
        if (out)
            return setFinished(Begin + 1, Pool);
        Pool.buffer.push_back(*Begin++);
    }
    // If we got this far, we are comparing to last character of "false".
    if (Begin == End)
        return setFinished(nullptr, Pool);
    if (*Begin != 'e')
        throw InvalidBool;
    return setFinished(Begin + 1, Pool);
    // If there is garbage afterwards, higher level parser will find it.
}
