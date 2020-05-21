// Format must be a const char array.
template<typename Sink, typename T, const auto& Format>
void WriteScalarType(Sink& S, T Value, std::vector<char>& Buffer) {
    int count = snprintf(&Buffer.front(), Buffer.size(), Format, Value);
    if (static_cast<int>(Buffer.size()) <= count) {
        Buffer.resize(count + 1);
        count = snprintf(&Buffer.front(), Buffer.size(), Format, Value);
    }
    S.write(&Buffer.front(), count);
}
