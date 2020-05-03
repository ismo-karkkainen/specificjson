template<typename Parser>
class ParseStdVector : public ParseArray<std::vector<typename Parser::Type>, Parser, false> { };
