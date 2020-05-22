extern const char* WriteInt64Format;
template<typename Sink>
void Write(Sink& S, std::int64_t Value, std::vector<char>& Buffer) {
    WriteScalarType<Sink,std::int64_t,WriteInt64Format>(S, Value, Buffer);
}
