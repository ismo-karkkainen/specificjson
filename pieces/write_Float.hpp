extern const Exception FloatNotFinite;
extern const char* WriteFloatFormat;

template<typename Sink>
void Write(Sink& S, float Value, std::vector<char>& Buffer) {
    if (isfinite(Value))
        WriteScalarType<Sink,float,WriteFloatFormat>(S, Value, Buffer);
    else
        throw FloatNotFinite;
}
