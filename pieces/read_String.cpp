static Exception StringStart("Expected '\"'.");
static Exception StringEscape("String with unknown escape.");
static Exception StringHexDigits("String with invalid hex digits.");
static Exception StringInvalidCharacter("String with invalid character.");

const char* specjson::ParseString::Parse(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    Type& out(std::get<Pool::Index>(Pool.Value));
    auto& buffer(Pool.buffer);
    if (finished) {
        if (*Begin != '"')
            throw StringStart;
        std::get<Pool::Index>(Pool.Value).resize(0);
        ++Begin;
    }
    while (Begin != End) {
        if (state == Normal) {
            while (Begin != End) {
                if (*Begin != '\\') {
                    if (*Begin != '"') {
                        if constexpr (static_cast<char>(0x80) < 0) {
                            // Signed char.
                            if (0 <= *Begin && *Begin < 32)
                                throw StringInvalidCharacter;
                        } else {
                            if (*Begin < 32)
                                throw StringInvalidCharacter;
                        }
                        buffer.push_back(*Begin++);
                    } else {
                        out.append(buffer.begin(), buffer.end());
                        return setFinished(Begin + 1, Pool);
                    }
                } else {
                    state = Escaped;
                    ++Begin;
                    break;
                }
            }
        } else if (state == Unicode) {
            hex_digits[count++] = *Begin++;
            if (count < 4)
                continue;
            count = 0;
            state = Normal;
            int value = 0;
            for (int k = 0; k < 4; ++k) {
                int m = 0;
                if ('0' <= hex_digits[k] && hex_digits[k] <= '9')
                    m = hex_digits[k] - '0';
                else if ('a' <= hex_digits[k] && hex_digits[k] <= 'f')
                    m = 10 + hex_digits[k] - 'a';
                else if ('A' <= hex_digits[k] && hex_digits[k] <= 'F')
                    m = 10 + hex_digits[k] - 'A';
                else
                    throw StringHexDigits;
                value = (value << 4) + m;
            }
            if (value < 0x80)
                buffer.push_back(static_cast<char>(value));
            else if (value < 0x800) {
                buffer.push_back(static_cast<char>(0xc0 | ((value >> 6) & 0x1f)));
                buffer.push_back(static_cast<char>(0x80 | (value & 0x3f)));
            } else {
                buffer.push_back(static_cast<char>(0xe0 | ((value >> 12) & 0xf)));
                buffer.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3f)));
                buffer.push_back(static_cast<char>(0x80 | (value & 0x3f)));
            }
        } else { // Escaped.
            state = Normal;
            switch (*Begin) {
            case '"':
            case '/':
            case '\\':
                buffer.push_back(*Begin);
                break;
            case 'b': buffer.push_back('\b'); break;
            case 'f': buffer.push_back('\f'); break;
            case 'n': buffer.push_back('\n'); break;
            case 'r': buffer.push_back('\r'); break;
            case 't': buffer.push_back('\t'); break;
            case 'u': state = Unicode; break;
            default:
                throw StringEscape;
            }
            ++Begin;
        }
    }
    return setFinished(nullptr, Pool);
}
