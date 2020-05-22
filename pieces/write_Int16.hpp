extern const char* WriteInt16Format;
template<typename Sink>
void Write(Sink& S, std::int16_t Value, std::vector<char>& Buffer) {
    WriteScalarType<Sink,std::int16_t,WriteInt16Format>(S, Value, Buffer);
}
