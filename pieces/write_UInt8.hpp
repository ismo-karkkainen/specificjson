extern const char* WriteUInt8Format;
template<typename Sink>
void Write(Sink& S, std::uint8_t Value, std::vector<char>& Buffer) {
    WriteScalarType<Sink,std::uint8_t,WriteUInt8Format>(S, Value, Buffer);
}
