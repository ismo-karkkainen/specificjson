template<typename Sink, typename ForwardIterator>
void Write(Sink& S,
    ForwardIterator& Begin, ForwardIterator& End, std::vector<char>& Buffer)
{
    char c = '[';
    S.write(&c, 1);
    if (Begin != End) {
        Write(S, *Begin, Buffer);
        c = ',';
        while (++Begin != End) {
            S.write(&c, 1);
            Write(S, *Begin, Buffer);
        }
    }
    c = ']';
    S.write(&c, 1);
}
