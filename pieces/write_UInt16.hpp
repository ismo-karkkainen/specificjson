extern const char* WriteUInt16Format;
template<typename Sink>
void Write(Sink& S, std::uint16_t Value, std::vector<char>& Buffer) {
    WriteScalarType<Sink,std::uint16_t,WriteUInt16Format>(S, Value, Buffer);
}
