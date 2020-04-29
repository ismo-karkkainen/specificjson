extern Exception InvalidKey;
extern Exception RequiredKeyNotGiven;
extern Exception InvalidObjectStart;
extern Exception InvalidKeySeparator;
extern Exception InvalidValueSeparator;


class ValueStore;

// Helper class for implementation of KeyValues template.
// Common interface for loops over tuple members etc.
class ScanningKeyValue {
protected:
    bool given;
    void Give(ValueStore* VS);

public:
    ScanningKeyValue() : given(false) { }
    virtual ~ScanningKeyValue();
    virtual ValueParser& Scanner(ParserPool& Pool) = 0;
    virtual const char* Key() const = 0;
    virtual void Swap(ValueStore* VS, ParserPool& Pool) = 0;
    virtual bool Required() const = 0;
    bool Given() const { return given; }
};

// Helper class for implementation of KeyValues template.
template<const char* KeyString, typename Parser>
class KeyValue : public ScanningKeyValue {
public:
    typedef typename Parser::Type Type;
    const char* Key() const { return KeyString; }
    void Swap(ValueStore* VS, ParserPool& Pool);

    void Swap(Type& Alt, ParserPool& Pool) {
        std::swap(Alt, std::get<Parser::Pool::Index>(Pool.Value));
        given = false;
    }

    ValueParser& Scanner(ParserPool& Pool) {
        given = true;
        return std::get<Parser::Pool::Index>(Pool.Parser);
    }

    bool Required() const { return false; }
};

// Helper class for implementation of KeyValues template.
template<const char* KeyString, typename Parser>
class RequiredKeyValue : public KeyValue<KeyString,Parser> {
public:
    bool Required() const { return true; }
};

// Helper class for implementation of KeyValues template.
template<const char* KeyString, typename Parser>
class KeyContainerValue : public ScanningKeyValue {
private:
    Parser p;

public:
    typedef typename Parser::Type Type;
    const char* Key() const { return KeyString; }
    void Swap(ValueStore* VS, ParserPool& Pool);

    void Swap(Type& Alt, ParserPool& Pool) {
        p.Swap(Alt);
        given = false;
    }

    ValueParser& Scanner(ParserPool& Pool) {
        given = true;
        return p;
    }

    bool Required() const { return false; }
};

// Helper class for implementation of KeyValues template.
template<const char* KeyString, typename Parser>
class RequiredKeyContainerValue : public KeyContainerValue<KeyString,Parser> {
    bool Required() const { return true; }
};


// Helper class for implementation of ParseObject class.
template<class ... Fields>
class KeyValues {
private:
    std::vector<ScanningKeyValue*> ptrs;

    template<typename FuncObj, typename Tuple, size_t... IdxSeq>
    void apply_to_each(FuncObj&& fo, Tuple&& t, std::index_sequence<IdxSeq...>)
    {
        int a[] = {
            (fo(std::get<IdxSeq>(std::forward<Tuple>(t))), void(), 0) ...
        };
        (void)a; // Suppresses unused variable warning.
    }

    class Pusher {
    private:
        std::vector<ScanningKeyValue*>& tgt;
    public:
        Pusher(std::vector<ScanningKeyValue*>& Target) : tgt(Target) { }
        void operator()(ScanningKeyValue& SKV) { tgt.push_back(&SKV); }
    };

public:
    std::tuple<Fields...> fields;

    KeyValues() {
        Pusher psh(ptrs);
        apply_to_each(psh, fields,
            std::make_index_sequence<std::tuple_size<decltype(fields)>::value>
                {});
    }

    size_t size() const { return std::tuple_size<decltype(fields)>::value; }
    ValueParser& Scanner(size_t Index, ParserPool& Pool) {
        return ptrs[Index]->Scanner(Pool);
    }
    ScanningKeyValue* KeyValue(size_t Index) { return ptrs[Index]; }
};

// Another derived class adds all the convenience methods that map to index.

// Base class for parse-time tracking of given keys and their values.
class ValueStore {
protected:
    bool given;

private:
    void Give();
    friend class ScanningKeyValue;

public:
    ValueStore() : given(false) { }
    bool Given() const { return given; }
};

template<typename Parser>
class Value : public ValueStore {
public:
    typedef typename Parser::Type Type;
    Type value;
};

template<class ... Fields>
class NamelessValues {
private:
    std::vector<ValueStore*> ptrs;

    template<typename FuncObj, typename Tuple, size_t... IdxSeq>
    void apply_to_each(FuncObj&& fo, Tuple&& t, std::index_sequence<IdxSeq...>)
    {
        int a[] = {
            (fo(std::get<IdxSeq>(std::forward<Tuple>(t))), void(), 0) ...
        };
        (void)a; // Suppresses unused variable warning.
    }

    class Pusher {
    private:
        std::vector<ValueStore*>& tgt;
    public:
        Pusher(std::vector<ValueStore*>& Target) : tgt(Target) { }
        void operator()(ValueStore& VS) { tgt.push_back(&VS); }
    };

public:
    std::tuple<Fields...> fields;

    NamelessValues() {
        Pusher psh(ptrs);
        apply_to_each(psh, fields, std::make_index_sequence<std::tuple_size<decltype(fields)>::value> {});
    }

    size_t size() const { return std::tuple_size<decltype(fields)>::value; }
    ValueStore* operator[](size_t Index) { return ptrs[Index]; }
};

template<const char* KeyString, typename Parser>
void KeyValue<KeyString,Parser>::Swap(ValueStore* VS, ParserPool& Pool) {
    Value<Parser>* dst(static_cast<Value<Parser>*>(VS));
    std::swap(dst->value, std::get<Parser::Pool::Index>(Pool.Value));
    Give(VS);
}

template<const char* KeyString, typename Parser>
void KeyContainerValue<KeyString,Parser>::Swap(ValueStore* VS, ParserPool& Pool)
{
    Value<Parser>* dst(static_cast<Value<Parser>*>(VS));
    p.Swap(dst->value);
    Give(VS);
}

// The Values class used by the template should be derived from NamelessValues
// and it adds way to access the fields using sensibly named methods.

template<typename KeyValues, typename Values>
class ParseObject : public ValueParser {
private:
    KeyValues parsers;
    Values out;
    int activating, active;
    enum State {
        NotStarted,
        PreKey,
        ExpectKey,
        PreColon,
        ExpectColon,
        PreValue,
        ExpectValue,
        PreComma,
        ExpectComma
    };
    State state;

    void setActivating(const std::string& Incoming) {
        for (size_t k = 0; k < parsers.size(); ++k)
            if (strcmp(Incoming.c_str(), parsers.KeyValue(k)->Key()) == 0) {
                activating = static_cast<int>(k);
                return;
            }
        throw InvalidKey;
    }

    const char* checkPassed(const char* Ptr) noexcept(false) {
        for (size_t k = 0; k < out.size(); ++k) {
            ScanningKeyValue* skv = parsers.KeyValue(k);
            ValueStore* vs = out[k];
            if (skv->Required() && !vs->Given())
                throw RequiredKeyNotGiven;
        }
        state = NotStarted;
        activating = -1;
        active = -1;
        return setFinished(Ptr);
    }

public:
    typedef decltype(out.fields) Type;

    ParseObject() : activating(-1), active(-1), state(NotStarted) { }

    const char* Parse(const char* Begin, const char* End, ParserPool& Pool)
        noexcept(false);

    void Swap(Type& Alt) {
        std::swap(Alt, out.fields);
        out.fields = Type();
    }
};

template<typename KeyValues, typename Values>
const char* ParseObject<KeyValues,Values>::Parse(
    const char* Begin, const char* End, ParserPool& Pool) noexcept(false)
{
    while (Begin != End) {
        switch (state) {
        case NotStarted:
            // Expect '{' on the first call.
            if (*Begin != '{')
                throw InvalidObjectStart;
            state = PreKey;
            ++Begin;
        case PreKey:
            if (isWhitespace(*Begin)) {
                Begin = skipWhitespace(++Begin, End);
                if (Begin == nullptr)
                    return setFinished(nullptr);
            }
            if (*Begin == '}')
                return checkPassed(++Begin);
            state = ExpectKey;
        case ExpectKey:
            Begin = std::get<ParserPool::String>(Pool.Parser).Parse(
                Begin, End, Pool);
            if (Begin == nullptr)
                return setFinished(nullptr);
            setActivating(std::get<ParserPool::String>(Pool.Value));
            state = PreColon;
        case PreColon:
            if (isWhitespace(*Begin)) {
                Begin = skipWhitespace(++Begin, End);
                if (Begin == nullptr)
                    return setFinished(nullptr);
            }
            state = ExpectColon;
        case ExpectColon:
            if (*Begin != ':')
                throw InvalidKeySeparator;
            state = PreValue;
            if (++Begin == End)
                return setFinished(nullptr);
        case PreValue:
            if (isWhitespace(*Begin)) {
                Begin = skipWhitespace(++Begin, End);
                if (Begin == nullptr)
                    return setFinished(nullptr);
            }
            active = activating;
            activating = -1;
            state = ExpectValue;
        case ExpectValue:
            Begin = parsers.Scanner(active, Pool).Parse(Begin, End, Pool);
            if (Begin == nullptr)
                return setFinished(nullptr);
            parsers.KeyValue(active)->Swap(out[active], Pool);
            active = -1;
            state = PreComma;
        case PreComma:
            if (isWhitespace(*Begin)) {
                Begin = skipWhitespace(++Begin, End);
                if (Begin == nullptr)
                    return setFinished(nullptr);
            }
            state = ExpectComma;
        case ExpectComma:
            if (*Begin == '}')
                return checkPassed(++Begin);
            if (*Begin != ',')
                throw InvalidValueSeparator;
            state = PreKey;
            ++Begin;
        }
    }
    return setFinished(nullptr);
}
