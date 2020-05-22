template<typename Sink>
void Write(Sink& S, bool Value, std::vector<char>& Buffer) {
    if (Value)
        S.write("true", 4);
    else
        S.write("false", 5);
}
