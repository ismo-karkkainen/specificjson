template<typename T>
static const char* floating_point_format(char* Buffer) {
    snprintf(Buffer, strlen(Buffer),
        "%%.%ig", std::numeric_limits<T>::digits10);
    return Buffer;
}
