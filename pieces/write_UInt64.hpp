extern const char* WriteUInt64Format;
template<typename Sink>
void Write(Sink& S, std::uint64_t Value, std::vector<char>& Buffer) {
    WriteScalarType<Sink,std::uint64_t,WriteUInt64Format>(S, Value, Buffer);
}
