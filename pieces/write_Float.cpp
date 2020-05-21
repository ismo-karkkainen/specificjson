const Exception specjson::FloatNotFinite("Float not finite.");
static char WriteFloatFormatBuffer[] = "1234567";
static const char* WriteFloatFormatContent =
    floating_point_format<float>(WriteFloatFormatBuffer);
const char* specjson::WriteFloatFormat = reinterpret_cast<const char*>(WriteFloatFormatBuffer);
