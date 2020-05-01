template<typename Parser>
class ParseContainerStdVector : public ParseContainerArray<std::vector<Parser::Type>, Parser> { };
