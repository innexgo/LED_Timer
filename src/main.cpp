#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Crypto.h>
#include <LittleFS.h>
#include <wpa2_enterprise.h>

#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

#define NUM_LEDS 60
#define DATA_PIN 13
#define BUTTON_PIN 5
#define DEBUG true


ESP8266WebServer server(80);

void handleInitPage(void){
}

void handleInit(void) {
  // Write to config file, keeping idle configs static.
}

void handleWifiConfig(void) {

}

void handleCredentialsConfig(void) {

}

void initStationWifi(void) {
  File wifi_station_file = LittleFS.open("wifi/station.txt", "r"); // Don't allow this to change to prevent this from becoming a fancy paperweight.
  int wifi_station_file_size = wifi_station_file.size();

  char wifi_station_chars[wifi_station_file_size + 1];
  byte size = wifi_station_file.readBytes(wifi_station_chars, wifi_station_file_size);

  wifi_station_chars[size] = 0; // set null character

  char *delimiters = " "; // Delimit with a space

  char *pch = strtok(wifi_station_chars , delimiters);

  char *wifi_station_values[2];

  /* Station file values:
  // 0: SSID
  // 1: Password
  */

  int pos = 0;

  while (pch != NULL) {
    wifi_station_values[pos] = pch;
    pos++;
    pch = strtok (0, delimiters);
  }

  WiFi.softAP(wifi_station_values[0], wifi_station_values[1], 1, 0, 1); // last two are default, but only allow one connector.
}

void initWifi(void) {
  File wifi_method_file = LittleFS.open("/wifi/method.txt", "r");
  String wifi_method_string = wifi_method_file.readString();
  wifi_method_file.close();
  const char *wifi_method = wifi_method_string.c_str();

  //TODO set up wifi methods.
  if (strcmp(wifi_method, "none") == 0) {
    Serial.println("Starting Internal Station");
    initStationWifi();
  }
  else if (strcmp(wifi_method, "WPA2") == 0) {
    Serial.println("Connecting to WiFi through WPA2");

    File wifi_SSID_file = LittleFS.open("/wifi/SSID.txt", "r");
    String wifi_SSID_string = wifi_SSID_file.readString();
    wifi_SSID_file.close();

    File wifi_pass_file = LittleFS.open("/wifi/pass.txt", "r");
    String wifi_pass_string = wifi_pass_file.readString();
    wifi_pass_file.close();

    WiFi.begin(wifi_SSID_string, wifi_pass_string);
  }
  else if (strcmp(wifi_method, "WPA2E") == 0) {
    Serial.println("Connecting to WiFi through WPA2E");

    File wifi_user_file = LittleFS.open("/wifi/user.txt", "r");
    
    int wifi_user_file_size = wifi_user_file.size();
    char wifi_user_chars[(wifi_user_file_size + 1)];
    byte user_size = wifi_user_file.readBytes(wifi_user_chars, wifi_user_file_size);
    wifi_user_chars[user_size] = 0;

    wifi_user_file.close();

    unsigned char* wifi_user_unsigned_chars = reinterpret_cast<unsigned char*>(wifi_user_chars);

    File wifi_pass_file = LittleFS.open("/wifi/pass.txt", "r");

    int wifi_pass_file_size = wifi_pass_file.size();
    char wifi_pass_chars[(wifi_pass_file_size + 1)];
    byte pass_size = wifi_pass_file.readBytes(wifi_pass_chars, wifi_pass_file_size);
    wifi_pass_chars[pass_size] = 0;

    wifi_pass_file.close();

    unsigned char* wifi_pass_unsigned_chars = reinterpret_cast<unsigned char*>(wifi_pass_chars);
        
    wifi_station_set_wpa2_enterprise_auth(1);
    wifi_station_set_enterprise_username(wifi_user_unsigned_chars, wifi_user_file_size);

  }
  else if (strcmp(wifi_method, "unsecured") == 0) {

    Serial.println("Connecting to Wifi through unencrypted air.");

    File wifi_SSID_file = LittleFS.open("/wifi/SSID.txt", "r");
    String wifi_SSID_string = wifi_SSID_file.readString();
    wifi_SSID_file.close();

    WiFi.begin(wifi_SSID_string);
  }
  else {
    Serial.print("Wifi Protocol Unrecognized: ");
    Serial.println(wifi_method);
    Serial.println("Starting Internal Station");
    initStationWifi();
  }
}

void handleLoginPage(void) {

}

void handleNotFound(void) {

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
    initStationWifi();
    server.on("/", handleInitPage);
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
}


void loop(void) {
  unsigned long current_ms = millis();

  readButton();
}