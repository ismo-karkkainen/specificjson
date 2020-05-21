extern const Exception DoubleNotFinite;
extern const char* WriteDoubleFormat;

template<typename Sink>
void Write(Sink& S, double Value, std::vector<char>& Buffer) {
    if (isfinite(Value))
        WriteScalarType<Sink,double,WriteDoubleFormat>(S, Value, Buffer);
    else
        throw DoubleNotFinite;
}
