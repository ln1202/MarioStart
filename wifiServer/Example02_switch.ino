#ifndef WIFI_INFO_H_
#define WIFI_INFO_H_

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiWebServer.h>
#endif



ESP8266WebServer server(80); // Web server on port 80

const char *init_ssid = "MarioStar_Setup";
const char *init_password = "12345678";

String home_ssid = "";
String home_password = "";

void startAPMode() {
    WiFi.softAP(init_ssid, init_password);
    Serial.println("Started AP Mode");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
    
    // Handle HTTP requests
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", R"rawliteral(
            <form action="/save" method="POST">
                <label>SSID: </label><input type="text" name="ssid"><br>
                <label>Password: </label><input type="password" name="password"><br>
                <button type="submit">Save</button>
            </form>
        )rawliteral");
    });

    server.on("/save", HTTP_POST, []() {
        if (server.hasArg("ssid") && server.hasArg("password")) {
            home_ssid = server.arg("ssid");
            home_password = server.arg("password");

            // Save Wi-Fi credentials to SPIFFS
            File file = SPIFFS.open("/wifi_config.txt", "w");
            if (file) {
                file.println(home_ssid);
                file.println(home_password);
                file.close();
            }

            server.send(200, "text/plain", "Settings saved! Rebooting...");
            delay(1000);
            ESP.restart();
        } else {
            server.send(400, "text/plain", "Missing SSID or Password");
        }
    });
    server.on("/reset", HTTP_POST, []() {
    SPIFFS.begin();
    if (SPIFFS.exists("/wifi_config.txt")) {
        SPIFFS.remove("/wifi_config.txt");  // 删除 Wi-Fi 配置文件
        server.send(200, "text/plain", "Wi-Fi configuration reset. Rebooting...");
        delay(1000);
        ESP.restart();  // 重启设备
    } else {
        server.send(400, "text/plain", "No Wi-Fi config to reset.");
    }
});

    server.begin();
    Serial.println("Web server started");
}

bool loadWiFiConfig() {
    if (!SPIFFS.begin()) {
        Serial.println("Failed to mount file system");
        return false;
    }

    if (!SPIFFS.exists("/wifi_config.txt")) {
        Serial.println("No Wi-Fi config found");
        return false;
    }

    File file = SPIFFS.open("/wifi_config.txt", "r");
    if (file) {
        home_ssid = file.readStringUntil('\n');
        home_ssid.trim();
        home_password = file.readStringUntil('\n');
        home_password.trim();
        file.close();
        return true;
    }

    return false;
}

void wifi_connect() {
    if (loadWiFiConfig()) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(home_ssid.c_str(), home_password.c_str());
        Serial.print("Connecting to ");
        Serial.println(home_ssid);

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWi-Fi connected");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("\nFailed to connect. Switching to AP mode.");
            startAPMode();
        }
    } else {
        startAPMode();
    }
}

#endif /* WIFI_INFO_H_ */
