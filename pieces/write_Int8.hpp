extern const char* WriteInt8Format;
template<typename Sink>
void Write(Sink& S, std::int8_t Value, std::vector<char>& Buffer) {
    WriteScalarType<Sink,std::int8_t,WriteInt8Format>(S, Value, Buffer);
}
