extern Exception FloatNotFinite;

template<typename Sink>
void Write(Sink& S, float Value, std::vector<char>& Buffer) {
    if (isfinite(Value)) {
        int count = snprintf(&Buffer.front(), Buffer.size(),
            "%.*g", std::numeric_limits<float>::digits10, double(Value));
        if (count < static_cast<int>(Buffer.size()))
            S.write(&Buffer.front(), count);
        else {
            Buffer.resize(count + 1);
            Write(S, Value, Buffer);
        }
    } else
        throw FloatNotFinite;
}
