extern const char* WriteIntFormat;
template<typename Sink>
void Write(Sink& S, int Value, std::vector<char>& Buffer) {
    WriteScalarType<Sink,int,WriteIntFormat>(S, Value, Buffer);
}
