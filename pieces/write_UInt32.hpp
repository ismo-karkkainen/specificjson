extern const char* WriteUInt32Format;
template<typename Sink>
void Write(Sink& S, std::uint32_t Value, std::vector<char>& Buffer) {
    WriteScalarType<Sink,std::uint32_t,WriteUInt32Format>(S, Value, Buffer);
}
