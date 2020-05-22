extern const char* WriteInt32Format;
template<typename Sink>
void Write(Sink& S, std::int32_t Value, std::vector<char>& Buffer) {
    WriteScalarType<Sink,std::int32_t,WriteInt32Format>(S, Value, Buffer);
}
