template<typename Sink>
void Write(Sink& S, int Value, std::vector<char>& Buffer) {
    int count = snprintf(&Buffer.front(), Buffer.size(), "%i", Value);
    if (count < static_cast<int>(Buffer.size()))
        S.write(&Buffer.front(), count);
    else {
        Buffer.resize(count + 1);
        Write(S, Value, Buffer);
    }
}
