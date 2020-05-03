template<typename Parser>
class ParseContainerStdVector : public ParseContainerArray<std::vector<typename Parser::Type>, Parser> { };
