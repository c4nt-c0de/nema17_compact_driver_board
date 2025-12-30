#include "OTA_handler.h"

OTA_handler::OTA_handler()
: server(80)
{}

void OTA_handler::begin(const char* ssid, const char* password,
                        std::function<void(void*)> register_routes,
                        const char* hostname)
{
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());

    // Start mDNS responder
#if defined(ESP8266)
    if (!MDNS.begin(hostname)) {
        Serial.println("Error starting mDNS");
    } else {
        Serial.print("mDNS responder started: http://");
        Serial.print(hostname);
        Serial.println(".local");
    }
#elif defined(ESP32)
    if (!MDNS.begin(hostname)) {
        Serial.println("Error starting mDNS");
    } else {
        Serial.print("mDNS responder started: http://");
        Serial.print(hostname);
        Serial.println(".local");
    }
#endif

    // ElegantOTA
    ElegantOTA.begin(&server);

    // Call user-provided route setup
    if(register_routes) register_routes(&server);

    server.begin();
    Serial.println("HTTP server started");
}

void OTA_handler::handle() {
    server.handleClient();
    ElegantOTA.loop();
}
