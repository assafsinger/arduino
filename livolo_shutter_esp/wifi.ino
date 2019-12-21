void initWiFi(void) {
#if DEBUG
  Serial.println(F("Start WiFi"));
#endif
  WiFi.persistent(true);
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED && _attempts <= MAX_WIFI_ATTEMPTS) {
      yield();
      delay(500);
#if DEBUG
      Serial.print(F("."));
#endif
      _attempts++;
    }
  }
#if DEBUG
  Serial.println();
  if (_attempts > MAX_WIFI_ATTEMPTS) {
    Serial.print(F("Failed to connect to "));
    Serial.println(SSID);
  } else {
    Serial.print(F("Connected to "));
    Serial.println(SSID);
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());
    Serial.print(F("Mac addresss: "));
    Serial.println(WiFi.macAddress());
  }
#endif
}