const Exception specjson::InvalidKey("Object, unexpected key.");
const Exception specjson::RequiredKeyNotGiven("Object, required key not given.");
const Exception specjson::InvalidObjectStart("Expected '{'");
const Exception specjson::InvalidKeySeparator("Object, expected ','");
const Exception specjson::InvalidValueSeparator("Object, expected ':'");

specjson::ScanningKeyValue::~ScanningKeyValue() { }

void specjson::ScanningKeyValue::Give(ValueStore* VS) {
    given = false;
    VS->Give();
}

void specjson::ValueStore::Give() {
    given = true;
}
