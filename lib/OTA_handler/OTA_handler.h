#pragma once

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
#endif

#include <ElegantOTA.h>
#include <functional>

class OTA_handler {
public:
    OTA_handler();

    // Optional: hostname parameter
    void begin(const char* ssid, const char* password,
               std::function<void(void*)> register_routes = nullptr,
               const char* hostname = "esp32"); // default hostname

    void handle();

#if defined(ESP8266)
    ESP8266WebServer server;
#elif defined(ESP32)
    WebServer server;
#endif
};
