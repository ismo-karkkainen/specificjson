template<typename Parser>
class ParseContainerStdVectorEqSize : public ParseContainerArray<std::vector<typename Parser::Type>, Parser, true> { };
