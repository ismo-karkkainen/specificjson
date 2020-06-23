ContextException::ContextException(const Exception& E,
    const char* Begin, const char* Current, const char* End)
    : Exception(context), context("")
{
    std::strncpy(context, E.what(), 42);
    context[41] = '\0';
    size_t space = (63 - strlen(context)) / 2;
    size_t before = (space < Current - Begin) ? space : (Current - Begin);
    size_t after = (space < End - Current) ? space : (End - Current);
    std::strncat(context, Current - before, before + after + 1);
}
