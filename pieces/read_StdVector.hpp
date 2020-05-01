template<typename Parser>
class ParseStdVector : public ParseArray<std::vector<Parser::Type>, Parser, false> { };
