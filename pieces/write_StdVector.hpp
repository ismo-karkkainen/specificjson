template<typename Sink, typename T>
void Write(Sink& S, const std::vector<T>& Value, std::vector<char>& Buffer) {
    auto b = Value.cbegin();
    auto e = Value.cend();
    Write<Sink,typename std::vector<T>::const_iterator>(S, b, e, Buffer);
}
