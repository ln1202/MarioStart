/*
 * switch.ino
 *
 *  Created on: 2020-05-15
 *      Author: Mixiaoxiao (Wang Bin)
 *
 * HAP section 8.38 Switch
 * An accessory contains a switch.
 *
 * This example shows how to:
 * 1. define a switch accessory and its characteristics (in my_accessory.c).
 * 2. get the switch-event sent from iOS Home APP.
 * 3. report the switch value to HomeKit.
 *
 * You should:
 * 1. read and use the Example01_TemperatureSensor with detailed comments
 *    to know the basic concept and usage of this library before other examples。
 * 2. erase the full flash or call homekit_storage_reset() in setup()
 *    to remove the previous HomeKit pairing storage and
 *    enable the pairing with the new accessory of this new HomeKit example.
 */

#include <Arduino.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"
#include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64



// #define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

void setup() {
	Serial.begin(115200);

   if (!SPIFFS.begin()) {
        Serial.println("Failed to initialize SPIFFS");
    }
  
	wifi_connect(); // in wifi_info.h
	//homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {
        my_homekit_setup(); // Only setup HomeKit in STA mode
    } else {
        Serial.println("HomeKit not started: in AP mode.");
    }
  // if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  //       Serial.println(F("SSD1306 allocation failed"));
  //       for(;;);
  //   }
    // display.clearDisplay();
    // display.setTextSize(1);      // 字體大小
    // display.setTextColor(SSD1306_WHITE); // 字體顏色
    // drawFilledStar(64, 40, 28);
    // display.display();
}

void loop() {
	 if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {
        my_homekit_loop();
    }

    // Handle web server in AP mode
    if (WiFi.getMode() == WIFI_AP) {
        server.handleClient();
    }

    delay(10); // Avoid busy loops
  
}

//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch_on;

static uint32_t next_heap_millis = 0;

#define PIN_SWITCH 2
#define PIN_D3 0

//Called when the switch value is changed by iOS Home APP
void cha_switch_on_setter(const homekit_value_t value) {
	bool on = value.bool_value;
	cha_switch_on.value.bool_value = on;	//sync the value
	LOG_D("Switch: %s", on ? "ON" : "OFF");
	digitalWrite(PIN_SWITCH, on ? LOW : HIGH);
  if (on) {
        // 當 switch 被打開時設定D3 pin
        digitalWrite(PIN_D3, LOW);   // PULL LOW
        delay(200);                  // 延遲 100 毫秒
        digitalWrite(PIN_D3, HIGH);  // PULL HIGH
    }
  cha_switch_on.value.bool_value = false;  // 更新 switch 狀態
  digitalWrite(PIN_SWITCH, HIGH);  // 設置 switch 為 HIGH (OFF)
  homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
}

void my_homekit_setup() {
	pinMode(PIN_SWITCH, OUTPUT);
	digitalWrite(PIN_SWITCH, HIGH);

  pinMode(PIN_D3, OUTPUT);         // 設定 D3 PIN模式
  digitalWrite(PIN_D3, HIGH);
	//Add the .setter function to get the switch-event sent from iOS Home APP.
	//The .setter should be added before arduino_homekit_setup.
	//HomeKit sever uses the .setter_ex internally, see homekit_accessories_init function.
	//Maybe this is a legacy design issue in the original esp-homekit library,
	//and I have no reason to modify this "feature".
	cha_switch_on.setter = cha_switch_on_setter;
	arduino_homekit_setup(&config);

	//report the switch value to HomeKit if it is changed (e.g. by a physical button)
	//bool switch_is_on = true/false;
	//cha_switch_on.value.bool_value = switch_is_on;
	//homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
}

void my_homekit_loop() {
  
	arduino_homekit_loop();
	const uint32_t t = millis();
	if (t > next_heap_millis) {
		// show heap info every 5 seconds
		next_heap_millis = t + 5 * 1000;
		LOG_D("Free heap: %d, HomeKit clients: %d",
				ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

	}
}
// void drawFilledStar(int16_t x0, int16_t y0, int16_t r) {
//   float angle = TWO_PI / 5;
//   float rotationOffset = 54 * PI / 180; //旋轉星星角度
//   int16_t x[5], y[5];

//   for (int i = 0; i < 5; i++) {
//     x[i] = x0 + r * cos(i * angle + rotationOffset);
//     y[i] = y0 + r * sin(i * angle + rotationOffset);
//   }

//   // 使用 fillTriangle 函数畫出五角星的五個三角形部分
//   for (int i = 0; i < 5; i++) {
//     display.fillTriangle(x0, y0, x[i], y[i], x[(i + 2) % 5], y[(i + 2) % 5], SSD1306_WHITE);
//   }
// }
