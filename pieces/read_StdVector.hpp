template<typename Parser>
class ParseStdVector : public ParseArray<std::vector<Parser::Type>, Parser, false> { };

template<typename Parser>
class ParseContainerStdVector : public ParseContainerArray<std::vector<Parser::Type>, Parser> { };
