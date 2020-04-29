extern Exception InvalidJSONCharacter;

template<typename Sink>
void Write(
    Sink& S, const char* Begin, const char* End, std::vector<char>& Buffer)
{
    char quote = '"';
    char escaped[3] = "\\ ";
    S.write(&quote, 1);
    for (const char* curr = Begin; curr < End; ++curr) {
        escaped[1] = 0;
        switch (*curr) {
        case '"': escaped[1] = '"'; break;
        case '\\': escaped[1] = '\\'; break;
        case '\n': escaped[1] = 'n'; break;
        case '\t': escaped[1] = 't'; break;
        case '\r': escaped[1] = 'r'; break;
        case '\f': escaped[1] = 'f'; break;
        case '\b': escaped[1] = 'b'; break;
        }
        if (escaped[1]) {
            S.write(Begin, curr - Begin);
            S.write(escaped, 2);
            Begin = curr + 1;
        } else {
            if constexpr (static_cast<char>(0x80) < 0) {
                if (0 <= *curr && *curr < 0x20)
                    throw InvalidJSONCharacter;
            } else {
                if (*curr < 0x20)
                    throw InvalidJSONCharacter;
            }
        }
    }
    if (Begin < End)
        S.write(Begin, End - Begin);
    S.write(&quote, 1);
}

template<typename Sink>
void Write(Sink& S, const std::string& Value, std::vector<char>& Buffer) {
    Write(S, Value.c_str(), Value.c_str() + Value.size(), Buffer);
}

template<typename Sink>
void Write(Sink& S, const char* Value, std::vector<char>& Buffer) {
    if (Value != nullptr)
        Write(S, Value, Value + strlen(Value), Buffer);
    else {
        char null[] = "null";
        S.write(null, 4);
    }
}
