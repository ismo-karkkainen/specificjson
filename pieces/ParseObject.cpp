Exception specjson::InvalidKey("Object, unexpected key.");
Exception specjson::RequiredKeyNotGiven("Object, required key not given.");
Exception specjson::InvalidObjectStart("Expected '{'");
Exception specjson::InvalidKeySeparator("Object, expected ','");
Exception specjson::InvalidValueSeparator("Object, expected ':'");

specjson::ScanningKeyValue::~ScanningKeyValue() { }

void specjson::ScanningKeyValue::Give(ValueStore* VS) {
    given = false;
    VS->Give();
}

void specjson::ValueStore::Give() {
    given = true;
}
