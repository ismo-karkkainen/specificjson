class Exception : public std::exception {
private:
    const char* reason;

public:
    Exception(const char* Reason) : reason(Reason) { }
    const char* what() const throw() { return reason; }
};
