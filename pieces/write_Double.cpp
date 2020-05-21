const Exception specjson::DoubleNotFinite("Double not finite.");
static char WriteDoubleFormatBuffer[] = "1234567";
static const char* WriteDoubleFormatContent =
    floating_point_format<double>(WriteDoubleFormatBuffer);
const char* specjson::WriteDoubleFormat = reinterpret_cast<const char*>(WriteDoubleFormatBuffer);
