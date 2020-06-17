#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Crypto.h>
#include <LittleFS.h>

#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

#include "wifi_select.h"

#define NUM_LEDS 60
#define DATA_PIN 13
#define BUTTON_PIN 5
#define DEBUG true


ESP8266WebServer server(80);

void handleCommonJS(void) {
  File common_js_file = LittleFS.open("/html/js/common.js", "r");
  String common_js_string = common_js_file.readString();
  common_js_file.close();

  const char *common_js_chars = common_js_string.c_str();

  server.send(200, "text/js", common_js_chars);
}

void handleInitPage(void) {
  File init_page_file = LittleFS.open("/html/init.html", "r");
  String init_page_string = init_page_file.readString();
  init_page_file.close();

  const char *init_page_chars = init_page_string.c_str();

  server.send(200, "text/html", init_page_chars);
}

void handleInitJS(void) {
  File init_js_file = LittleFS.open("/html/js/init.js", "r");
  String init_js_string = init_js_file.readString();
  init_js_file.close();

  const char *init_js_chars = init_js_string.c_str();

  server.send(200, "text/js", init_js_chars);
}

void handleWifiConfig(void) {

}

void handleCredentialsConfig(void) {

}

void handleLoginPage(void) {

}

void handleCredentialLogin(void) {

}

void handleNotFound(void) {
  server.send(404, "text/plain", "404, Resource not Found.");
}

void readButton(void) {
  if(digitalRead(BUTTON_PIN) == LOW) {
    // TODO Reset stuff
    LittleFS.remove("/config.txt"); // Ensure the file is removed.
    LittleFS.remove("/pass.txt");
    LittleFS.remove("/wifi/user.txt");
    LittleFS.remove("/wifi/pass.txt");
    LittleFS.remove("/wifi/SSID.txt");
    File config_file = LittleFS.open("/config.txt", "w");
    config_file.write("False, True, 424242");
    config_file.close();
  }
}

void setup(void) {
  if (DEBUG) {
    Serial.begin(115200);
  }
  LittleFS.begin();
  server.onNotFound(handleNotFound);

  pinMode(BUTTON_PIN, INPUT);

  CRGB leds[NUM_LEDS];
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  File config_file = LittleFS.open("/config.txt", "r");
  int config_size = config_file.size();

  char config_chars[config_size + 1];
  byte size = config_file.readBytes(config_chars, config_size);

  config_file.close();

  config_chars[size] = 0;

  char *delimiters = " ,.-";

  char *pch = strtok(config_chars , delimiters);

  char *config_values[3];

  /* Config values:
  // 0: Init 
  // 1: Idle behavior
  // 2: Idle color
  */

  int pos = 0;

  while (pch != NULL) {
    config_values[pos] = pch;
    pos++;
    pch = strtok (0, delimiters);
  }
  
  char *true_ptr = "True";

  // Check if init done.
  if (strcmp(config_values[0], true_ptr) == 0) {
    Serial.println("CV0: True");
    server.on("/", handleLoginPage);
    initWifi();
  }
  else {
    Serial.println("CV0: False");
    // Init code.
    // initStationWifi(); //TODO Replace this, taken out for testing.
    server.on("/", handleInitPage);
    server.on("/init.html", handleInitPage);
    server.on("/js/init.js", handleInitJS);
    server.on("/wifi", handleWifiConfig);
    server.on("/credentials", handleCredentialsConfig);
  }

  // Set Default Idle color.
  if (strcmp(config_values[1], true_ptr) == 0) {
    Serial.println("CV1: True");
    int color = std::strtol(config_values[2], 0, 16);
    fill_solid(leds, 60, CRGB(color));
    FastLED.show();
  }
  else {
    Serial.println("CV1: False");
    fill_solid(leds, 60, CRGB(0,0,0));
  }

  if (DEBUG) {
    Serial.println("Starting WiFi");
    initWifi();
  }

  server.on("/js/common.js", handleCommonJS);

  server.begin();
}


void loop(void) {
  unsigned long current_ms = millis();

  server.handleClient();
  readButton();
}