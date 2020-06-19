#include <LittleFS.h>
#include <ESP8266Wifi.h>
#include <ESP8266mDNS.h>
#include <wpa2_enterprise.h>

#include "wifi_select.h"

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

void setHostname(void) {
  File hostname_file = LittleFS.open("/wifi/hostname.txt", "r");
  String hostname_string = hostname_file.readString();
  hostname_file.close();

  const char *hostname_chars = hostname_string.c_str();

  WiFi.hostname(hostname_chars);
  Serial.printf("Hostname: %s\n", WiFi.hostname().c_str());

  if (MDNS.begin(hostname_string)) {
    Serial.println("MDNS responder started");
  }
}

void initWifi(void) {
  WiFi.softAPdisconnect();
  setHostname();

  File wifi_method_file = LittleFS.open("/wifi/method.txt", "r");
  String wifi_method_string = wifi_method_file.readString();
  wifi_method_file.close();
  const char *wifi_method = wifi_method_string.c_str();

  if (strcmp(wifi_method, "none") == 0) {
    Serial.println("Starting Internal Station");
    initStationWifi();
  }
  else if (strcmp(wifi_method, "WPA2") == 0) {
    Serial.println("Connecting to WiFi through WPA2");

    File wifi_SSID_file = LittleFS.open("/wifi/SSID.txt", "r");
    String wifi_SSID_string = wifi_SSID_file.readString();
    wifi_SSID_file.close();
    
    const char *ssid_chars = wifi_SSID_string.c_str();
    Serial.printf("Through SSID: %s\n", ssid_chars);

    File wifi_pass_file = LittleFS.open("/wifi/pass.txt", "r");
    String wifi_pass_string = wifi_pass_file.readString();
    wifi_pass_file.close();

    WiFi.begin(wifi_SSID_string, wifi_pass_string);

    int count = 0;
    while (WiFi.status() != WL_CONNECTED) { //Purposefully blocking.
      count++;
      delay(500);
      Serial.print(".");
      if (count > 120) {
        Serial.println("Failed to connect to WiFi, starting internal AP.");
        initStationWifi();
        break;
      }
    }
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

    File wifi_SSID_file = LittleFS.open("/wifi/SSID.txt", "r");
    String wifi_SSID_string = wifi_SSID_file.readString();
    wifi_SSID_file.close();

    const char *ssid_chars = wifi_SSID_string.c_str();
    struct station_config stationConf;
    os_memcpy(&stationConf.ssid, ssid_chars, 32);

    Serial.printf("Through SSID: %s\n", ssid_chars);
    
    wifi_station_set_config(&stationConf);
    wifi_station_set_wpa2_enterprise_auth(1);
    wifi_station_set_enterprise_username(wifi_user_unsigned_chars, wifi_user_file_size);
    wifi_station_set_enterprise_password(wifi_pass_unsigned_chars, wifi_pass_file_size);

    wifi_station_connect();
    int count = 0;
    while (wifi_station_get_connect_status()){ //Purposefully blocking.
      count++;
      delay(500);
      Serial.print(".");
      if (count > 120) {
        Serial.println("Failed to connect to WiFi, starting internal AP.");
        initStationWifi();
        break;
      }
    }
  }
  else if (strcmp(wifi_method, "unsecured") == 0) {
    Serial.println("Connecting to Wifi through unencrypted air.");

    File wifi_SSID_file = LittleFS.open("/wifi/SSID.txt", "r");
    String wifi_SSID_string = wifi_SSID_file.readString();
    wifi_SSID_file.close();

    const char *ssid_chars = wifi_SSID_string.c_str();
    Serial.printf("Through SSID: %s\n", ssid_chars);

    WiFi.begin(wifi_SSID_string);
    int count = 0;
    while (WiFi.status() != WL_CONNECTED) { //Purposefully blocking.
      count++;
      delay(500);
      Serial.print(".");
      if (count > 120) {
        Serial.println("Failed to connect to WiFi, starting internal AP.");
        initStationWifi();
        break;
      }
    }
  }
  else {
    Serial.print("Wifi Protocol Unrecognized: ");
    Serial.println(wifi_method);
    Serial.println("Starting Internal Station");
    initStationWifi();
  }
  Serial.println();
  Serial.println("WiFi Started");
}