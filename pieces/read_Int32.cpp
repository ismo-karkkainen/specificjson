const char* specjson::ParseInt32::Parse(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    const char* end = parse_integer<Type,
        std::numeric_limits<Type>::lowest(), std::numeric_limits<Type>::max()>(
            std::get<ParseInt32::Pool::Index>(Pool.Value),
            Begin, End, Pool.buffer, finished);
    return setFinished(end, Pool);
}
