class Exception : public std::exception {
private:
    const char* reason;

public:
    Exception(const char* Reason) : reason(Reason) { }
    const char* what() const throw() { return reason; }
};


class ContextException : public Exception {
private:
    char context[64];

public:
    ContextException(const Exception& E,
        const char* Begin, const char* Current, const char* End);
};
