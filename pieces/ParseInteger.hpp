extern const Exception InvalidInt;
extern const Exception IntOutsideRange;

template<typename T, intmax_t Minimum, uintmax_t Maximum>
T convert_to_integer(const char* Begin, char** end) noexcept(false) {
    errno = 0;
    if constexpr (Minimum < 0) {
        intmax_t out = strtoimax(Begin, end, 10);
        if (errno == ERANGE)
            throw IntOutsideRange;
        if (out < Minimum || (0 < out && Maximum < out))
            throw IntOutsideRange;
        return static_cast<T>(out);
    } else {
        if (*Begin == '-')
            throw IntOutsideRange; // Avoid returning negation of value.
        uintmax_t out = strtoumax(Begin, end, 10);
        if (errno == ERANGE)
            throw IntOutsideRange;
        if (out < Minimum || Maximum < out)
            throw IntOutsideRange;
        return static_cast<T>(out);
    }
}

template<typename T, intmax_t Minimum, uintmax_t Maximum>
const char* parse_integer(T& Out, const char* Begin, const char* End,
    std::vector<char>& Buffer, bool Finished)
    noexcept(false)
{
    char* end = nullptr;
    if (Finished) {
        Out = convert_to_integer<T,Minimum,Maximum>(Begin, &end);
        if (end == Begin) {
            if (Begin == End)
                return nullptr;
            // Unless only sign fit, throw.
            if (Begin + 1 != End || (*Begin != '-' && *Begin != '+'))
                throw InvalidInt;
        } else if (end != End)
            return end; // Good up to this.
        while (Begin != End)
            Buffer.push_back(*Begin++);
        return nullptr;
    }
    // Start of the number is in buffer. Input is null-terminated.
    while (('0' <= *Begin && *Begin <= '9') || *Begin == '-' || *Begin == '+')
        Buffer.push_back(*Begin++);
    if (Begin == End) // Continues on and on?
        return nullptr;
    Buffer.push_back(0);
    Out = convert_to_integer<T,Minimum,Maximum>(&Buffer.front(), &end);
    // Separator scan will throw if the string in source is not a number.
    // Require that all chars are the number as there was no separator copied.
    if (end != &Buffer.back())
        throw InvalidInt;
    return Begin;
}
