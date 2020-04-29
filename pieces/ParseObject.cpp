Exception InvalidKey("Object, unexpected key.");
Exception RequiredKeyNotGiven("Object, required key not given.");
Exception InvalidObjectStart("Expected '{'");
Exception InvalidKeySeparator("Object, expected ','");
Exception InvalidValueSeparator("Object, expected ':'");

ScanningKeyValue::~ScanningKeyValue() { }

void ScanningKeyValue::Give(ValueStore* VS) {
    given = false;
    VS->Give();
}

void ValueStore::Give() {
    given = true;
}
