#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Crypto.h>
#include <LittleFS.h>
#include <regex.h>


#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

#include "wifi_select.h"

#define NUM_LEDS 60
#define DATA_PIN 13
#define BUTTON_PIN 5
#define DEBUG true

String default_color = "686868";

ESP8266WebServer server(80);

void handleCommonCSS(void) {
  File common_css_file = LittleFS.open("/html/css/common.css", "r"); // These file operations have to be seperate because
  String common_css_string = common_css_file.readString();           // as far as I know, mutex's don't exist in arduino
  const char *common_css_chars = common_css_string.c_str();
  common_css_file.close();

  server.send(200, "text/css", common_css_chars);
}

void handleCommonJS(void) {
  File common_js_file = LittleFS.open("/html/js/common.js", "r");
  String common_js_string = common_js_file.readString();
  const char *common_js_chars = common_js_string.c_str();
  common_js_file.close();

  server.send(200, "text/javascript", common_js_chars);
}

void appendFoundNetworks(int networksFound) {
  Serial.printf("%d network(s) found\n", networksFound);

  File foundNetworks = LittleFS.open("/html/js/foundNetworks.js", "w");
  foundNetworks.write("var selection = document.getElementById('wifi-selection');\n");

  for (int i = 0; i < networksFound; i++)
  {
    String SSID = WiFi.SSID(i);
    Serial.printf("%d: %s, Ch:%d (%ddBm)\n", i + 1, SSID.c_str(), WiFi.channel(i), WiFi.RSSI(i));

    foundNetworks.write("var option = document.createElement('option');\n");
    foundNetworks.write("option.innerText = '");

    const char *network_SSID = SSID.c_str();
    foundNetworks.write(network_SSID);

    String network_type;
    if (WiFi.encryptionType(i) == ENC_TYPE_CCMP) {
      network_type = "WPA2";
    }
    else if (WiFi.encryptionType(i) == ENC_TYPE_NONE) {
      network_type = "unsecured";
    }
    else {
      network_type = "auto";
    }
    const char *network_type_char = network_type.c_str();
    foundNetworks.write("';\noption.value = '");
    foundNetworks.write(network_type_char);
    foundNetworks.write("';\nselection.add(option);");
  }
  foundNetworks.close();
}

void handleInitPage(void) {
  File init_page_file = LittleFS.open("/html/init.html", "r");
  String init_page_string = init_page_file.readString();
  const char *init_page_chars = init_page_string.c_str();
  init_page_file.close();

  server.send(200, "text/html", init_page_chars);
}

void handleInitJS(void) {
  File init_js_file = LittleFS.open("/html/js/init.js", "r");
  String init_js_string = init_js_file.readString();
  const char *init_js_chars = init_js_string.c_str();
  init_js_file.close();

  server.send(200, "text/javascript", init_js_chars);
}

void handleFoundNetworksJS(void) {
  File found_networks_js_file = LittleFS.open("/html/js/foundNetworks.js", "r");
  String found_networks_js_string = found_networks_js_file.readString();
  const char *found_networks_js_chars = found_networks_js_string.c_str();
  found_networks_js_file.close();

  server.send(200, "text/javascript", found_networks_js_chars);
}

void handleInitCSS(void) {
  File init_css_file = LittleFS.open("/html/css/init.css", "r");
  String init_css_string = init_css_file.readString();
  const char *init_css_chars = init_css_string.c_str();
  init_css_file.close();

  server.send(200, "text/css", init_css_chars);
}

void handleWifiConfig(void) {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  }

  for (uint8_t i = 0; i < server.args(); i++) {
    Serial.printf("%s ", server.argName(i).c_str());
    Serial.println(server.arg(i));;

    if (server.argName(i) == "SSID") {
      int length = strlen(server.arg(i).c_str());

      if (length >= 250) {
        server.send(400, "text/plain", "SSID way too long. Stop sending fake requests.");
        return;
      }

      File wifi_SSID = LittleFS.open("/wifi/SSID.txt", "w");
      const char *SSID_chars = server.arg(i).c_str();
      wifi_SSID.write(SSID_chars);
      wifi_SSID.close();
    }

    if (server.argName(i) == "user") {
      int length = strlen(server.arg(i).c_str());

      if (length >= 250) {
        server.send(400, "text/plain", "Username way too long. Try a shorter one");
        return;
      }

      File wifi_user = LittleFS.open("/wifi/user.txt", "w");
      const char *user_chars = server.arg(i).c_str();
      wifi_user.write(user_chars);
      wifi_user.close();
    }

    if (server.argName(i) == "pass") {
      int length = strlen(server.arg(i).c_str());

      if (length >= 250) {
        server.send(400, "text/plain", "Password way too long. Try a shorter one");
        return;
      }

      File wifi_pass = LittleFS.open("/wifi/pass.txt", "w");
      const char *pass_chars = server.arg(i).c_str();
      wifi_pass.write(pass_chars);
      wifi_pass.close();
    }

    if (server.argName(i) == "type") {
      String conn_type = server.arg(i);

      if (!(conn_type == "none" || conn_type == "WPA2" || conn_type == "WPA2E" || conn_type == "unsecured")) {
        server.send(400, "text/plain", "That option doesn't exist, stop sending fake requests.");
        return;
      }

      File wifi_conn_type = LittleFS.open("/wifi/method.txt", "w");
      const char *conn_type_chars = server.arg(i).c_str();
      wifi_conn_type.write(conn_type_chars);
      wifi_conn_type.close();
    }
  }

  server.send(200, "text/plain", "Valid Data Recieved");

}

void handleCredentialsConfig(void) {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  }

  if (!LittleFS.exists("/wifi/method.txt")) {
    server.send(412, "text/plain", "Precondition not met, wifi connection type not chosen.");
    return;
  }

  for (uint8_t i = 0; i < server.args(); i++) {
    Serial.printf("%s ", server.argName(i).c_str());
    Serial.println(server.arg(i));

    if (server.argName(i) == "name") {
      int length = strlen(server.arg(i).c_str());

      if (length >= 250) {
        server.send(400, "text/plain", "Hostname way too long.");
        return;
      }

      if (length < 6) {
        server.send(400, "text/plain", "Hostname must be greater than 6 characters long.");
        return;
      }
      
      regex_t name_pattern;
      int ret_int;
      char hostname[length+1];

      ret_int = regcomp(&name_pattern, "^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9-]*[a-zA-Z0-9])\\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9-]*[A-Za-z0-9])$", REG_EXTENDED);
      if (ret_int) {
        Serial.println("Could not compile regex");
        return;
      }

      strcpy(hostname, server.arg(i).c_str());

      ret_int = regexec(&name_pattern, hostname, 0, NULL, 0);

      regfree(&name_pattern);

      if (!ret_int) {
        Serial.println("Hostname input valid");
        File hostname_file = LittleFS.open("/wifi/hostname.txt", "w");
        const char *hostname_chars = server.arg(i).c_str();
        hostname_file.write(hostname_chars);
        hostname_file.close();
      }
      else if (ret_int == REG_NOMATCH) {
        Serial.println("Hostname input NOT valid");
        server.send(400, "text/plain", "Invalid Hostname");
        return;
      }
      else {
        Serial.print("Regex Failed at Hostname");
        server.send(500, "text/plain", "Internal Server Error, Error 2");
        return;
      }
    }

    if (server.argName(i) == "pass") {
      int length = strlen(server.arg(i).c_str());

      if (length >= 250) {
        server.send(400, "text/plain", "Password way too long.");
        return;
      }

      if (length < 6) {
        server.send(400, "text/plain", "Password must be greater than 6 characters long.");
        return;
      }

      File pass_file = LittleFS.open("/pass.txt", "w");
      const char *pass_chars = server.arg(i).c_str();
      pass_file.write(pass_chars);
      pass_file.close();
    }
  }

  server.send(200, "text/plain", "Valid Data Recieved");

  File config_file = LittleFS.open("/config.txt", "w");
  const char *to_write = default_color.c_str();
  config_file.write("True, True, ");
  config_file.write(to_write);;
  config_file.close();

  Serial.printf("Wrote to /config.txt: %s\n", to_write);

  delay(200);

  void(* resetFunc) (void) = 0;//declare reset function at address 0

  Serial.println("Restarting");
  resetFunc();
}

void handleLoginPage(void) {
  File login_page_file = LittleFS.open("/html/login.html", "r");
  String login_page_string = login_page_file.readString();
  const char *login_page_chars = login_page_string.c_str();
  login_page_file.close();

  server.send(200, "text/html", login_page_chars);
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
    const char *to_write = default_color.c_str();
    config_file.write("True, True, ");
    config_file.write(to_write);
    config_file.close();

    Serial.printf("Wrote to /config.txt: %s\n", to_write);
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
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.println("Scanning Networks");
    delay(100);
    WiFi.scanNetworksAsync(appendFoundNetworks, false);
    while (WiFi.scanComplete() == -1) {
      delay(250);
      Serial.print(".");
    }
    Serial.println("");
    server.on("/", handleInitPage);
    server.on("/init.html", handleInitPage);
    server.on("/js/init.js", handleInitJS);
    server.on("/js/foundNetworks.js", handleFoundNetworksJS);
    server.on("/css/init.css", handleInitCSS);
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

  Serial.println("Starting WiFi");
  initWifi();

  server.on("/js/common.js", handleCommonJS);
  server.on("/css/common.css", handleCommonCSS);
  server.on("/favicon.ico", handleNotFound);

  server.begin();
}


void loop(void) {
  server.handleClient();

  unsigned long current_ms = millis();

  readButton();
}