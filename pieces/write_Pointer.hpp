template<typename Sink, typename T>
void Write(Sink& S, const T* Value, std::vector<char>& Buffer) {
    if (Value != nullptr)
        Write(S, *Value, Buffer);
    else {
        char null[] = "null";
        S.write(&null, 4);
    }
}
