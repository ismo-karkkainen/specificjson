const char* specjson::ParseUInt64::Parse(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    const char* end = parse_integer<Type,
        std::numeric_limits<Type>::lowest(), std::numeric_limits<Type>::max()>(
            std::get<ParseUInt64::Pool::Index>(Pool.Value),
            Begin, End, Pool.buffer, finished);
    return setFinished(end, Pool);
}
