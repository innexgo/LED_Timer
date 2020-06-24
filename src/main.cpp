#include <stdlib.h>
#include <sstream>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <regex.h>
#include <ESP8266TrueRandom.h>
#include <ESP8266Ping.h>


#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

#include "wifi_select.h"
#include "Crypto.h"

#define NUM_LEDS 60
#define DATA_PIN 13
#define BUTTON_PIN 5
#define DEBUG true
#define UTC_OFFSET 2208988800ULL
#define NTP_OFFSET   0            // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds

CRGB leds[NUM_LEDS];

String default_color = "686868";
boolean has_internet = false;

boolean timer_on = false;
boolean add_time = false;
long millis_offset = 0;
int lit_LED_count = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", NTP_OFFSET, NTP_INTERVAL);

ESP8266WebServer server(80);


void initTimer(void) {
}

void setTimer(void) {
}

void timerAdd(void) {
}


void changeIdle(void) {
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

  // Set Default Idle color.
  if (strcmp(config_values[1], "true") == 0) {
    Serial.println("Idle Color: True");
    int color = std::strtol(config_values[2], 0, 16);
    fill_solid(leds, 60, CRGB(color));
    FastLED.show();
  }
  else {
    Serial.println("Idle Color: False");
    fill_solid(leds, 60, CRGB(0));
    FastLED.show();
  }
}

void handleNotFound(void) {
  server.send(404, "text/plain", "404, Resource not Found.");
}

void handleFavicon(void) {
  server.send(204, "", "");
}


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

void handleInitCSS(void) {
  File init_css_file = LittleFS.open("/html/css/init.css", "r");
  String init_css_string = init_css_file.readString();
  const char *init_css_chars = init_css_string.c_str();
  init_css_file.close();

  server.send(200, "text/css", init_css_chars);
}

void handleFoundNetworksJS(void) {
  File found_networks_js_file = LittleFS.open("/html/js/foundNetworks.js", "r");
  String found_networks_js_string = found_networks_js_file.readString();
  const char *found_networks_js_chars = found_networks_js_string.c_str();
  found_networks_js_file.close();

  server.send(200, "text/javascript", found_networks_js_chars);
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
  config_file.write("true true ");
  config_file.write(to_write);;
  config_file.close();

  Serial.println("Wrote to /config.txt:");

  delay(200);

  void(* resetFunc) (void) = 0;//declare reset function at address 0

  Serial.println("Restarting");
  resetFunc();
}


void generateVerificationJSFile(void) {
  File verif_js_file = LittleFS.open("/html/js/verification.js", "w");
  verif_js_file.write("var verif_div = document.getElementById('verification');\nverif_div.innerText = '");
  File verif_file = LittleFS.open("/verif.txt", "r");
  String verif_str = verif_file.readString();
  const char *verif_chars = verif_str.c_str();
  verif_js_file.write(verif_chars);
  verif_file.close();
  verif_js_file.write("';");
  verif_js_file.close();
}

void generateVerification(void) {
  ESP8266TrueRandomClass random_gen;

  int gen_length = random_gen.random(20, 50);
  char verif_buf[gen_length];
  
  random_gen.memfill(&verif_buf[0], gen_length);

  SHA256 hasher;
  hasher.doUpdate(verif_buf, gen_length);
  byte hash[SHA256_SIZE];
  hasher.doFinal(hash);

  Serial.printf("Length of Verif Data: %d\n", gen_length);

  //Code taken from Serial.print()
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *hash_str = &buf[sizeof(buf) - 1];

  *hash_str = '\0';

  // Code taken from Serial.print() and modified to work for my scenario.
  // Apparently the original doesn't fully work, probably edge case.
  for (int i = (SHA256_SIZE-1); i >= 0; i--) {
    unsigned long hash_long = (unsigned long)hash[i];
    for (int j = 0; j <= 1; j++ ) {
      unsigned long m = hash_long;
      hash_long /= HEX;
      char c = m - HEX * hash_long;
      *--hash_str = (c < 10 ? (c + '0') : c + 'A' - 10);
    }
  }
  Serial.printf("%s\n", hash_str);

  char hash_char_str[65];
  hash_char_str[65] = 0;
  strcpy(hash_char_str, hash_str);
  File verification_file = LittleFS.open("verif.txt", "w");
  verification_file.write((const char*)hash_char_str);
  verification_file.close();

  generateVerificationJSFile();
}

void handleVerificationJS(void) {
  File verif_js_file = LittleFS.open("/html/js/verification.js", "r");
  String verif_js_string = verif_js_file.readString();
  const char *verif_js_chars = verif_js_string.c_str();
  verif_js_file.close();

  server.send(200, "text/javascript", verif_js_chars);
}


void handleHostname(void) {
  File hostname_file = LittleFS.open("/wifi/hostname.txt", "r");
  String hostname_string = hostname_file.readString();
  const char *hostname_chars = hostname_string.c_str();
  hostname_file.close();

  server.send(200, "text/html", hostname_chars);
}

void handleLoginPage(void) {
  File login_page_file = LittleFS.open("/html/login.html", "r");
  String login_page_string = login_page_file.readString();
  const char *login_page_chars = login_page_string.c_str();
  login_page_file.close();

  server.send(200, "text/html", login_page_chars);
}

void handleLoginJS(void) {
  File login_js_file = LittleFS.open("/html/js/login.js", "r");
  String login_js_string = login_js_file.readString();
  const char *login_js_chars = login_js_string.c_str();
  login_js_file.close();

  server.send(200, "text/javascript", login_js_chars);
}

void handleSCJL(void) {
  File sjcl_js_file = LittleFS.open("/html/js/sjcl.js", "r");
  String sjcl_js_string = sjcl_js_file.readString();
  server.send(200, "text/javascript", (sjcl_js_string.c_str()));
  sjcl_js_file.close();
}


void handleCredentialLogin(void) {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  }

  const char *inc_hash_str;
  unsigned long inc_time = 0; // incoming time, should work for the next few hundred years.
  const char *inc_time_str; // keeping it as a string as well for easier processing later

  for (uint8_t i = 0; i < server.args(); i++) {
    Serial.printf("%s ", server.argName(i).c_str());
    Serial.println(server.arg(i));;

    if (server.argName(i) == "hash") {
      int length = strlen(server.arg(i).c_str());

      if (length <= 63) {
        server.send(400, "text/plain", "Hash way too short. Stop sending fake requests.");
        return;
      }
      if (length >= 65) {
        server.send(400, "text/plain", "Hash way too long. Stop sending fake requests.");
        return;
      }

      inc_hash_str = server.arg(i).c_str();
    }
    
    if (server.argName(i) == "time") {
      int length = strlen(server.arg(i).c_str());

      if (length < 10) {
        server.send(400, "text/plain", "You're in the future or the past, how is it there? What's the new protocol used?\
         Anyways, if you're from this decade, 2020's, stop the fake requests.");
        return;
      }

      if (length >= 25) {
        server.send(400, "text/plain", "Time way too long. Stop sending fake requests.");
        return;
      }

      inc_time_str = (server.arg(i).c_str());
      
      // Because the normal functions failed me for over 5 hours, I am forced to do this.
      // Loads of ASCII casting pointing converting black magic concieved at 6 am after an allnighter.
      // Perk is that it divides by 1000 by ignoring the last 3 positions.
      for (int pos = (length-4); pos >= 0; pos --) {
        inc_time += (int)((((char)*(inc_time_str++)) - '0') * ((int)pow(10, pos)));
      }

      inc_time_str = (server.arg(i).c_str()); // Need to replace the pointer b/c of previous operation moving it, at least if my understanding is sound.
    }
  }

  if (has_internet) {
    unsigned long sys_epoch_time =  timeClient.getEpochTime();

    Serial.printf("Cur Time: %ld\n", sys_epoch_time);
    Serial.printf("Inc Time: %ld\n", (long int)inc_time); 
    // cast to normal long to allow for negatives
    if (((long)(sys_epoch_time - inc_time)) > 15) { 
      server.send(400, "text/plain", "Took over 15 seconds to send, or you're sending fake requests.");
      return;
    }

    if (((long)(sys_epoch_time - inc_time)) < (-5)) { 
      server.send(400, "text/plain", "You sent this request over 5 seconds in the past, stop sending fake requests.");
      return;
    }
  }

  if (inc_time == 0) {
    server.send(400, "text/plain", "Stop the fake requests.");
    return;
  }

  if (strcmp(inc_hash_str, "") == 0) {
    server.send(400, "text/plain", "Stop the fake requests.");
    return;
  }

  SHA256 hasher;
  
  //An attempt to deallocate some memory first.
  if (true) {
    Serial.printf("Adding time:  %s\n", inc_time_str);
    hasher.doUpdate(inc_time_str);

    File verif_file = LittleFS.open("/verif.txt", "r");
    String verif_str = verif_file.readString();
    const char *verif_chars = verif_str.c_str();
    verif_file.close();

    Serial.printf("Adding verif: %s\n", verif_chars);
    hasher.doUpdate(verif_chars);

    File pass_file = LittleFS.open("/pass.txt", "r");
    String pass_str = pass_file.readString();
    const char *pass_chars = pass_str.c_str();
    pass_file.close();

    Serial.println("Adding pass"); //  %s\n", pass_chars);
    hasher.doUpdate(pass_chars);
  }

  byte calc_hash[SHA256_SIZE];
  hasher.doFinal(calc_hash);


  //Code taken from Serial.print()
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *hash_str = &buf[sizeof(buf) - 1];

  *hash_str = '\0';

  // Code taken from Serial.print() and modified to work for my scenario.
  // Apparently the original doesn't fully work, probably edge case.
  for (int i = (SHA256_SIZE-1); i >= 0; i--) {
    unsigned long hash_long = (unsigned long)calc_hash[i];
    for (int j = 0; j <= 1; j++ ) {
      unsigned long m = hash_long;
      hash_long /= HEX;
      char c = m - HEX * hash_long;
      *--hash_str = (c < 10 ? (c + '0') : c + 'A' - 10);
    }
  }


  Serial.printf("Incoming Hash:   %s\n", inc_hash_str);
  Serial.printf("Calculated Hash: %s\n", hash_str);

  ESP.wdtFeed(); // Before watchdog bites me.

  if (strcmp(hash_str, inc_hash_str) == 0) {
    Serial.println("Password matches");
    server.send(200, "text/plain", "Password Good.");
    return;
  }
  else {
    Serial.println("Password do not match");
    server.send(401, "text/plain", "Wrong password.");
    return;
  }
}


void handleGetTimer(void) {
  File end_time_file = LittleFS.open("/timer/end_time.txt", "r");
  String end_time_string = end_time_file.readString();
  const char *end_time_chars = end_time_string.c_str();
  end_time_file.close();

  server.send(200, "text/html", end_time_chars);
}


void handleTimerPage(void) {
  File timer_page_file = LittleFS.open("/html/timer.html", "r");
  String timer_page_string = timer_page_file.readString();
  const char *timer_page_chars = timer_page_string.c_str();
  timer_page_file.close();

  server.send(200, "text/html", timer_page_chars);
}

void handleTimerJS(void) {
  File timer_js_file = LittleFS.open("/html/js/timer.js", "r");
  String timer_js_string = timer_js_file.readString();
  const char *timer_js_chars = timer_js_string.c_str();
  timer_js_file.close();

  server.send(200, "text/javascript", timer_js_chars);
}


void handleSetTimer(void) {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  }

  const char *inc_hash_str;
  unsigned long inc_time = 0; // incoming time, should work for the next few hundred years.
  const char *inc_time_str; // keeping it as a string as well for easier processing later
  unsigned long end_time = 0;
  const char *end_time_str;
  unsigned long duration_time = 0;
  const char *duration_str;
  const char *subtle = "";

  for (uint8_t i = 0; i < server.args(); i++) {
    Serial.printf("%s ", server.argName(i).c_str());
    Serial.println(server.arg(i));;

    if (server.argName(i) == "hash") {
      int length = strlen(server.arg(i).c_str());

      if (length <= 63) {
        server.send(400, "text/plain", "Hash way too short. Stop sending fake requests.");
        return;
      }
      if (length >= 65) {
        server.send(400, "text/plain", "Hash way too long. Stop sending fake requests.");
        return;
      }

      inc_hash_str = (server.arg(i).c_str());
    }

    if (server.argName(i) == "subtle") {
      if (((strcmp((server.arg(i).c_str()), "true")) != 0) && ((strcmp((server.arg(i).c_str()), "false")) != 0)) {
        server.send(400, "text/plain", "Invalid value for field: subtle");
        return;
      }

      subtle = (server.arg(i).c_str());
    }
    
    if (server.argName(i) == "time") {
      int length = strlen(server.arg(i).c_str());

      if (length < 10) {
        server.send(400, "text/plain", "You're in the future, how is it there? What's the new protocol used?\
         Anyways, if you're from this decade, 2020's, stop the fake requests.");
        return;
      }

      if (length >= 25) {
        server.send(400, "text/plain", "Time way too long. Stop sending fake requests.");
        return;
      }

      inc_time_str = (server.arg(i).c_str());
      
      // Because the normal functions failed me for over 5 hours, I am forced to do this.
      // Loads of ASCII casting pointing converting black magic concieved at 6 am after an allnighter.
      // Perk is that it divides by 1000 by ignoring the last 3 positions.
      for (int pos = (length-4); pos >= 0; pos --) {
        inc_time += (int)((((char)*(inc_time_str++)) - '0') * ((int)pow(10, pos)));
      }

      inc_time_str = (server.arg(i).c_str()); // Need to replace the pointer b/c of previous operation moving it, at least if my understanding is sound.
    }

    if (server.argName(i) == "end") {
      int length = strlen(server.arg(i).c_str());

      if (length < 10) {
        server.send(400, "text/plain", "You're in the future, how is it there? What's the new protocol used?\
         Anyways, if you're from this decade, 2020's, stop the fake requests.");
        return;
      }

      if (length >= 25) {
        server.send(400, "text/plain", "Time way too long. Stop sending fake requests.");
        return;
      }

      end_time_str = (server.arg(i).c_str());
      
      // Because the normal functions failed me for over 5 hours, I am forced to do this.
      // Loads of ASCII casting pointing converting black magic concieved at 6 am after an allnighter.
      // No need to div by 1k, already done.
      for (int pos = (length-1); pos >= 0; pos --) {
        end_time += (int)((((char)*(end_time_str++)) - '0') * ((int)pow(10, pos)));
      }

      end_time_str = (server.arg(i).c_str()); // Need to replace the pointer b/c of previous operation moving it, at least if my understanding is sound.
    }

    if (server.argName(i) == "duration") {
      int length = strlen(server.arg(i).c_str());

      if (length >= 25) {
        server.send(400, "text/plain", "Time way too long. Stop sending fake requests.");
        return;
      }
      
      duration_str = (server.arg(i).c_str());
      
      // Because the normal functions failed me for over 5 hours, I am forced to do this.
      // Loads of ASCII casting pointing converting black magic concieved at 6 am after an allnighter.
      // No need to div by 1k, already done.
      for (int pos = (length-1); pos >= 0; pos --) {
        duration_time += (int)((((char)*(duration_str++)) - '0') * ((int)pow(10, pos)));
      }

      if (duration_time < 5) {
        server.send(400, "text/plain", "Bad request.");
        return;
      }

      duration_str = (server.arg(i).c_str()); // Need to replace the pointer b/c of previous operation moving it, at least if my understanding is sound.
    }
  }

  if (has_internet) {
    unsigned long sys_epoch_time =  timeClient.getEpochTime();

    Serial.printf("Cur Time: %ld\n", sys_epoch_time);
    Serial.printf("Inc Time: %ld\n", (long int)inc_time); 
    // cast to normal long to allow for negatives
    if (((long)(sys_epoch_time - inc_time)) > 15) { 
      server.send(400, "text/plain", "Took over 15 seconds to send, or you're sending fake requests.");
      return;
    }

    if (((long)(sys_epoch_time - inc_time)) < (-5)) { 
      server.send(400, "text/plain", "You sent this request over 5 seconds in the past, stop sending fake requests.");
      return;
    }
  }


  SHA256 hasher;
  
  //An attempt to deallocate some memory first.
  if (true) {
    Serial.printf("Adding time:  %s\n", inc_time_str);
    hasher.doUpdate(inc_time_str);

    Serial.printf("Adding end_t: %s\n", end_time_str);
    hasher.doUpdate(end_time_str);

    Serial.printf("Adding dur_t: %s\n", duration_str);
    hasher.doUpdate(duration_str);

    if (strcmp(subtle, "") != 0) {
      Serial.printf("Adding subt: %s\n", subtle);
      hasher.doUpdate(subtle);
    }

    File verif_file = LittleFS.open("/verif.txt", "r");
    String verif_str = verif_file.readString();
    const char *verif_chars = verif_str.c_str();
    verif_file.close();

    Serial.printf("Adding verif: %s\n", verif_chars);
    hasher.doUpdate(verif_chars);

    File pass_file = LittleFS.open("/pass.txt", "r");
    String pass_str = pass_file.readString();
    const char *pass_chars = pass_str.c_str();
    pass_file.close();

    Serial.println("Adding pass"); //  %s\n", pass_chars);
    hasher.doUpdate(pass_chars);
  }

  byte calc_hash[SHA256_SIZE];
  hasher.doFinal(calc_hash);


  //Code taken from Serial.print()
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *hash_str = &buf[sizeof(buf) - 1];

  *hash_str = '\0';

  // Code taken from Serial.print() and modified to work for my scenario.
  // Apparently the original doesn't fully work, probably edge case.
  for (int i = (SHA256_SIZE-1); i >= 0; i--) {
    unsigned long hash_long = (unsigned long)calc_hash[i];
    for (int j = 0; j <= 1; j++ ) {
      unsigned long m = hash_long;
      hash_long /= HEX;
      char c = m - HEX * hash_long;
      *--hash_str = (c < 10 ? (c + '0') : c + 'A' - 10);
    }
  }


  Serial.printf("Incoming Hash:   %s\n", inc_hash_str);
  Serial.printf("Calculated Hash: %s\n", hash_str);

  ESP.wdtFeed(); // Before watchdog bites me.

  if (strcmp(hash_str, inc_hash_str) == 0) {
    Serial.println("Password matches");
    server.send(200, "text/plain", "Password Good.");
  }
  else {
    Serial.println("Password do not match");
    server.send(401, "text/plain", "Wrong password.");
    return;
  }
  
  if (strcmp(subtle, "") == 0) {
    File duration_file = LittleFS.open("/timer/duration.txt", "w");
    Serial.printf("Wrote to duration file: %s\n", duration_str);
    duration_file.write(duration_str);
    duration_file.close();

    File end_time_file = LittleFS.open("/timer/end_time.txt", "w");
    Serial.printf("Wrote to end_time file: %s\n", end_time_str);
    end_time_file.write(end_time_str);
    end_time_file.close();

    File add_time_file = LittleFS.open("/timer.add.txt", "w");
    add_time_file.write("");
    add_time_file.close();
  }
  else {
    String to_write = "";
    to_write += subtle;
    to_write += " ";
    to_write += duration_str;
    to_write += " ";
    to_write += end_time_str;

    const char *to_write_char = (to_write.c_str());
    Serial.printf("Writing to add time file: %s\n", to_write_char);
    File add_time_file = LittleFS.open("/timer/add.txt", "w");
    add_time_file.write(to_write_char);
    add_time_file.close();
  }
}


void handleGetSettings(void) {
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

  String send_string = "";
  send_string += config_values[1];
  send_string += " ";
  send_string += config_values[2];

  File warning_file = LittleFS.open("/timer/warn.txt", "r");
  String warning_string = warning_file.readString();
  warning_file.close();

  send_string += " ";
  send_string += warning_string;

  File norm_color_file = LittleFS.open("/timer/norm_color.txt", "r");
  String norm_color_string = norm_color_file.readString();
  norm_color_file.close();

  send_string += " ";
  send_string += norm_color_string;

  File warn_color_file = LittleFS.open("/timer/warn_color.txt", "r");
  String warn_color_string = warn_color_file.readString();
  warn_color_file.close();

  send_string += " ";
  send_string += warn_color_string;

  File stop_color_file = LittleFS.open("/timer/stop_color.txt", "r");
  String stop_color_string = stop_color_file.readString();
  stop_color_file.close();

  send_string += " ";
  send_string += stop_color_string;

  const char* send_string_char = send_string.c_str();
  Serial.printf("Settings: %s\n", send_string_char);
  server.send(200, "text/html", send_string_char);
}


void handleSettingsPage(void) {
  File settings_page_file = LittleFS.open("/html/settings.html", "r");
  String settings_page_string = settings_page_file.readString();
  const char *settings_page_chars = settings_page_string.c_str();
  settings_page_file.close();

  server.send(200, "text/html", settings_page_chars);
}

void handleSettingsJS(void) {
  File settings_js_file = LittleFS.open("/html/js/settings.js", "r");
  String settings_js_string = settings_js_file.readString();
  const char *settings_js_chars = settings_js_string.c_str();
  settings_js_file.close();

  server.send(200, "text/javascript", settings_js_chars);
}


void handleSetOpColors(void) {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  }

  const char *inc_hash_str;
  unsigned long inc_time = 0; // incoming time, should work for the next few hundred years.
  const char *inc_time_str; // keeping it as a string as well for easier processing later
  const char *norm_color;
  const char *warn_color;
  const char *stop_color;

  for (uint8_t i = 0; i < server.args(); i++) {
    Serial.printf("%s ", server.argName(i).c_str());
    Serial.println(server.arg(i));;

    if (server.argName(i) == "hash") {
      int length = strlen(server.arg(i).c_str());

      if (length <= 63) {
        server.send(400, "text/plain", "Hash way too short. Stop sending fake requests.");
        return;
      }
      if (length >= 65) {
        server.send(400, "text/plain", "Hash way too long. Stop sending fake requests.");
        return;
      }

      inc_hash_str = (server.arg(i).c_str());
    }
    
    if (server.argName(i) == "time") {
      int length = strlen(server.arg(i).c_str());

      if (length < 10) {
        server.send(400, "text/plain", "You're in the future, how is it there? What's the new protocol used?\
         Anyways, if you're from this decade, 2020's, stop the fake requests.");
        return;
      }

      if (length >= 25) {
        server.send(400, "text/plain", "Time way too long. Stop sending fake requests.");
        return;
      }

      inc_time_str = (server.arg(i).c_str());
      
      // Because the normal functions failed me for over 5 hours, I am forced to do this.
      // Loads of ASCII casting pointing converting black magic concieved at 6 am after an allnighter.
      // Perk is that it divides by 1000 by ignoring the last 3 positions.
      for (int pos = (length-4); pos >= 0; pos --) {
        inc_time += (int)((((char)*(inc_time_str++)) - '0') * ((int)pow(10, pos)));
      }

      inc_time_str = (server.arg(i).c_str()); // Need to replace the pointer b/c of previous operation moving it, at least if my understanding is sound.
    }

    if (server.argName(i) == "norm-color") {
      int length = strlen(server.arg(i).c_str());

      if (length > 6) {
        server.send(400, "text/plain", "Time way too long. Stop sending fake requests.");
        return;
      }

      if (length < 6) {
        server.send(400, "text/plain", "Color too short/not valid");
        return;
      }

      norm_color = (server.arg(i).c_str());

      regex_t color_pattern;
      int ret_int;
      char color[length+1];

      ret_int = regcomp(&color_pattern, "^([0-9A-Za-z]{6})$", REG_EXTENDED);
      if (ret_int) {
        Serial.println("Could not compile regex");
        return;
      }

      strcpy(color, server.arg(i).c_str());

      ret_int = regexec(&color_pattern, color, 0, NULL, 0);

      regfree(&color_pattern);

      if (!ret_int) {
        Serial.println("color input valid");
      }
      else if (ret_int == REG_NOMATCH) {
        Serial.println("Color input NOT valid");
        server.send(400, "text/plain", "Invalid Normal Operating Color");
        return;
      }
      else {
        Serial.print("Regex Failed at Color");
        server.send(500, "text/plain", "Internal Server Error, Error 6");
        return;
      }
    }

    if (server.argName(i) == "warn-color") {
      int length = strlen(server.arg(i).c_str());

      if (length > 6) {
        server.send(400, "text/plain", "Time way too long. Stop sending fake requests.");
        return;
      }

      if (length < 6) {
        server.send(400, "text/plain", "Color too short/not valid");
        return;
      }

      warn_color = (server.arg(i).c_str());

      regex_t color_pattern;
      int ret_int;
      char color[length+1];

      ret_int = regcomp(&color_pattern, "^([0-9A-Za-z]{6})$", REG_EXTENDED);
      if (ret_int) {
        Serial.println("Could not compile regex");
        return;
      }

      strcpy(color, server.arg(i).c_str());

      ret_int = regexec(&color_pattern, color, 0, NULL, 0);

      regfree(&color_pattern);

      if (!ret_int) {
        Serial.println("color input valid");
      }
      else if (ret_int == REG_NOMATCH) {
        Serial.println("Color input NOT valid");
        server.send(400, "text/plain", "Invalid Warning Color");
        return;
      }
      else {
        Serial.print("Regex Failed at Color");
        server.send(500, "text/plain", "Internal Server Error, Error 6");
        return;
      }
    }

    if (server.argName(i) == "stop-color") {
      int length = strlen(server.arg(i).c_str());

      if (length > 6) {
        server.send(400, "text/plain", "Time way too long. Stop sending fake requests.");
        return;
      }

      if (length < 6) {
        server.send(400, "text/plain", "Color too short/not valid");
        return;
      }

      stop_color = (server.arg(i).c_str());

      regex_t color_pattern;
      int ret_int;
      char color[length+1];

      ret_int = regcomp(&color_pattern, "^([0-9A-Za-z]{6})$", REG_EXTENDED);
      if (ret_int) {
        Serial.println("Could not compile regex");
        return;
      }

      strcpy(color, server.arg(i).c_str());

      ret_int = regexec(&color_pattern, color, 0, NULL, 0);

      regfree(&color_pattern);

      if (!ret_int) {
        Serial.println("color input valid");
      }
      else if (ret_int == REG_NOMATCH) {
        Serial.println("Color input NOT valid");
        server.send(400, "text/plain", "Invalid Stop Color");
        return;
      }
      else {
        Serial.print("Regex Failed at Color");
        server.send(500, "text/plain", "Internal Server Error, Error 7");
        return;
      }
    }

  }

  if (has_internet) {
    unsigned long sys_epoch_time =  timeClient.getEpochTime();

    Serial.printf("Cur Time: %ld\n", sys_epoch_time);
    Serial.printf("Inc Time: %ld\n", (long int)inc_time); 
    // cast to normal long to allow for negatives
    if (((long)(sys_epoch_time - inc_time)) > 15) { 
      server.send(400, "text/plain", "Took over 15 seconds to send, or you're sending fake requests.");
      return;
    }

    if (((long)(sys_epoch_time - inc_time)) < (-5)) { 
      server.send(400, "text/plain", "You sent this request over 5 seconds in the past, stop sending fake requests.");
      return;
    }
  }

  if (strcmp(inc_hash_str, "") == 0) {
    server.send(400, "text/plain", "Stop the fake requests.");
    return;
  }


  SHA256 hasher;
  
  //An attempt to deallocate some memory first.
  if (true) {
    Serial.printf("Adding cur time:   %s\n", inc_time_str);
    hasher.doUpdate(inc_time_str);

    Serial.printf("Adding norm color: %s\n", norm_color);
    hasher.doUpdate(norm_color);

    Serial.printf("Adding warn color: %s\n", warn_color);
    hasher.doUpdate(warn_color);

    Serial.printf("Adding stop color: %s\n", stop_color);
    hasher.doUpdate(stop_color);

    File verif_file = LittleFS.open("/verif.txt", "r");
    String verif_str = verif_file.readString();
    const char *verif_chars = verif_str.c_str();
    verif_file.close();

    Serial.printf("Adding verif str:  %s\n", verif_chars);
    hasher.doUpdate(verif_chars);

    File pass_file = LittleFS.open("/pass.txt", "r");
    String pass_str = pass_file.readString();
    const char *pass_chars = pass_str.c_str();
    pass_file.close();

    Serial.println("Adding password"); //  %s\n", pass_chars);
    hasher.doUpdate(pass_chars);
  }

  byte calc_hash[SHA256_SIZE];
  hasher.doFinal(calc_hash);


  //Code taken from Serial.print()
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *hash_str = &buf[sizeof(buf) - 1];

  *hash_str = '\0';

  // Code taken from Serial.print() and modified to work for my scenario.
  // Apparently the original doesn't fully work, probably edge case.
  for (int i = (SHA256_SIZE-1); i >= 0; i--) {
    unsigned long hash_long = (unsigned long)calc_hash[i];
    for (int j = 0; j <= 1; j++ ) {
      unsigned long m = hash_long;
      hash_long /= HEX;
      char c = m - HEX * hash_long;
      *--hash_str = (c < 10 ? (c + '0') : c + 'A' - 10);
    }
  }


  Serial.printf("Incoming Hash:   %s\n", inc_hash_str);
  Serial.printf("Calculated Hash: %s\n", hash_str);

  ESP.wdtFeed(); // Before watchdog bites me.

  if (strcmp(hash_str, inc_hash_str) == 0) {
    Serial.println("Password matches");
    server.send(200, "text/plain", "Password Good.");
  }
  else {
    Serial.println("Password do not match");
    server.send(401, "text/plain", "Wrong password.");
    return;
  }

  Serial.printf("Writing to norm color file: %s\n", norm_color);
  File norm_color_file = LittleFS.open("/timer/norm_color.txt", "w");
  norm_color_file.write(norm_color);
  norm_color_file.close();

  Serial.printf("Writing to warn color file: %s\n", warn_color);
  File warn_color_file = LittleFS.open("/timer/warn_color.txt", "w");
  warn_color_file.write(warn_color);
  warn_color_file.close();

  Serial.printf("Writing to stop color file: %s\n", stop_color);
  File stop_color_file = LittleFS.open("/timer/stop_color.txt", "w");
  stop_color_file.write(stop_color);
  stop_color_file.close();
}

void handleSetWarning(void) {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  }

  const char *inc_hash_str;
  unsigned long inc_time = 0; // incoming time, should work for the next few hundred years.
  const char *inc_time_str; // keeping it as a string as well for easier processing later
  unsigned long warn_time = 0;
  const char *warn_time_str;
  const char *enabled;

  for (uint8_t i = 0; i < server.args(); i++) {
    Serial.printf("%s ", server.argName(i).c_str());
    Serial.println(server.arg(i));;

    if (server.argName(i) == "hash") {
      int length = strlen(server.arg(i).c_str());

      if (length <= 63) {
        server.send(400, "text/plain", "Hash way too short. Stop sending fake requests.");
        return;
      }
      if (length >= 65) {
        server.send(400, "text/plain", "Hash way too long. Stop sending fake requests.");
        return;
      }

      inc_hash_str = (server.arg(i).c_str());
    }
    
    if (server.argName(i) == "time") {
      int length = strlen(server.arg(i).c_str());

      if (length < 10) {
        server.send(400, "text/plain", "You're in the future, how is it there? What's the new protocol used?\
         Anyways, if you're from this decade, 2020's, stop the fake requests.");
        return;
      }

      if (length >= 25) {
        server.send(400, "text/plain", "Time way too long. Stop sending fake requests.");
        return;
      }

      inc_time_str = (server.arg(i).c_str());
      
      // Because the normal functions failed me for over 5 hours, I am forced to do this.
      // Loads of ASCII casting pointing converting black magic concieved at 6 am after an allnighter.
      // Perk is that it divides by 1000 by ignoring the last 3 positions.
      for (int pos = (length-4); pos >= 0; pos --) {
        inc_time += (int)((((char)*(inc_time_str++)) - '0') * ((int)pow(10, pos)));
      }

      inc_time_str = (server.arg(i).c_str()); // Need to replace the pointer b/c of previous operation moving it, at least if my understanding is sound.
    }

    if (server.argName(i) == "warn-time") {
      int length = strlen(server.arg(i).c_str());

      if (length >= 25) {
        server.send(400, "text/plain", "Time way too long. Stop sending fake requests.");
        return;
      }

      warn_time_str = (server.arg(i).c_str());
      
      // Because the normal functions failed me for over 5 hours, I am forced to do this.
      // Loads of ASCII casting pointing converting black magic concieved at 6 am after an allnighter.
      // No need to div by 1k, already done.
      for (int pos = (length-1); pos >= 0; pos --) {
        warn_time += (int)((((char)*(warn_time_str++)) - '0') * ((int)pow(10, pos)));
      }

      warn_time_str = (server.arg(i).c_str()); // Need to replace the pointer b/c of previous operation moving it, at least if my understanding is sound.
    }
 
    if (server.argName(i) == "enabled") {
      int length = strlen(server.arg(i).c_str());

      if (((strcmp((server.arg(i).c_str()), "true")) != 0) && ((strcmp((server.arg(i).c_str()), "false")) != 0)) {
        server.send(400, "text/plain", "Invalid value for field: enabled");
        return;
      }

      enabled = (server.arg(i).c_str());
    }
  }

  if (has_internet) {
    unsigned long sys_epoch_time =  timeClient.getEpochTime();

    Serial.printf("Cur Time: %ld\n", sys_epoch_time);
    Serial.printf("Inc Time: %ld\n", (long int)inc_time); 
    // cast to normal long to allow for negatives
    if (((long)(sys_epoch_time - inc_time)) > 15) { 
      server.send(400, "text/plain", "Took over 15 seconds to send, or you're sending fake requests.");
      return;
    }

    if (((long)(sys_epoch_time - inc_time)) < (-5)) { 
      server.send(400, "text/plain", "You sent this request over 5 seconds in the past, stop sending fake requests.");
      return;
    }
  }


  if (strcmp(inc_hash_str, "") == 0) {
    server.send(400, "text/plain", "Stop the fake requests.");
    return;
  }


  SHA256 hasher;
  
  //An attempt to deallocate some memory first.
  if (true) {
    Serial.printf("Adding time:   %s\n", inc_time_str);
    hasher.doUpdate(inc_time_str);

    Serial.printf("Adding warn_t: %s\n", warn_time_str);
    hasher.doUpdate(warn_time_str);

    Serial.printf("Adding enb_d:  %s\n", enabled);
    hasher.doUpdate(enabled);

    File verif_file = LittleFS.open("/verif.txt", "r");
    String verif_str = verif_file.readString();
    const char *verif_chars = verif_str.c_str();
    verif_file.close();

    Serial.printf("Adding verif:  %s\n", verif_chars);
    hasher.doUpdate(verif_chars);

    File pass_file = LittleFS.open("/pass.txt", "r");
    String pass_str = pass_file.readString();
    const char *pass_chars = pass_str.c_str();
    pass_file.close();

    Serial.println("Adding pass"); //  %s\n", pass_chars);
    hasher.doUpdate(pass_chars);
  }

  byte calc_hash[SHA256_SIZE];
  hasher.doFinal(calc_hash);


  //Code taken from Serial.print()
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *hash_str = &buf[sizeof(buf) - 1];

  *hash_str = '\0';

  // Code taken from Serial.print() and modified to work for my scenario.
  // Apparently the original doesn't fully work, probably edge case.
  for (int i = (SHA256_SIZE-1); i >= 0; i--) {
    unsigned long hash_long = (unsigned long)calc_hash[i];
    for (int j = 0; j <= 1; j++ ) {
      unsigned long m = hash_long;
      hash_long /= HEX;
      char c = m - HEX * hash_long;
      *--hash_str = (c < 10 ? (c + '0') : c + 'A' - 10);
    }
  }


  Serial.printf("Incoming Hash:   %s\n", inc_hash_str);
  Serial.printf("Calculated Hash: %s\n", hash_str);

  ESP.wdtFeed(); // Before watchdog bites me.

  if (strcmp(hash_str, inc_hash_str) == 0) {
    Serial.println("Password matches");
    server.send(200, "text/plain", "Password Good.");
  }
  else {
    Serial.println("Password do not match");
    server.send(401, "text/plain", "Wrong password.");
    return;
  }
  
  String to_write = "";
  to_write += enabled;
  to_write += " ";
  to_write += warn_time_str;

  const char *to_write_char = (to_write.c_str());
  Serial.printf("Writing to warn time file: %s\n", to_write_char);
  File warn_time_file = LittleFS.open("/timer/warn.txt", "w");
  warn_time_file.write(to_write_char);
  warn_time_file.close();
}

void handleSetIdle(void) {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  }

  const char *inc_hash_str;
  unsigned long inc_time = 0; // incoming time, should work for the next few hundred years.
  const char *inc_time_str; // keeping it as a string as well for easier processing later
  const char *idle_color;
  const char *enabled;

  for (uint8_t i = 0; i < server.args(); i++) {
    Serial.printf("%s ", server.argName(i).c_str());
    Serial.println(server.arg(i));;

    if (server.argName(i) == "hash") {
      int length = strlen(server.arg(i).c_str());

      if (length <= 63) {
        server.send(400, "text/plain", "Hash way too short. Stop sending fake requests.");
        return;
      }
      if (length >= 65) {
        server.send(400, "text/plain", "Hash way too long. Stop sending fake requests.");
        return;
      }

      inc_hash_str = (server.arg(i).c_str());
    }
    
    if (server.argName(i) == "time") {
      int length = strlen(server.arg(i).c_str());

      if (length < 10) {
        server.send(400, "text/plain", "You're in the future, how is it there? What's the new protocol used?\
         Anyways, if you're from this decade, 2020's, stop the fake requests.");
        return;
      }

      if (length >= 25) {
        server.send(400, "text/plain", "Time way too long. Stop sending fake requests.");
        return;
      }

      inc_time_str = (server.arg(i).c_str());
      
      // Because the normal functions failed me for over 5 hours, I am forced to do this.
      // Loads of ASCII casting pointing converting black magic concieved at 6 am after an allnighter.
      // Perk is that it divides by 1000 by ignoring the last 3 positions.
      for (int pos = (length-4); pos >= 0; pos --) {
        inc_time += (int)((((char)*(inc_time_str++)) - '0') * ((int)pow(10, pos)));
      }

      inc_time_str = (server.arg(i).c_str()); // Need to replace the pointer b/c of previous operation moving it, at least if my understanding is sound.
    }

    if (server.argName(i) == "idle-color") {
      int length = strlen(server.arg(i).c_str());

      if (length >= 25) {
        server.send(400, "text/plain", "Time way too long. Stop sending fake requests.");
        return;
      }

      idle_color = (server.arg(i).c_str());

      regex_t color_pattern;
      int ret_int;
      char color[length+1];

      ret_int = regcomp(&color_pattern, "^([0-9A-Za-z]{6})$", REG_EXTENDED);
      if (ret_int) {
        Serial.println("Could not compile regex");
        return;
      }

      strcpy(color, server.arg(i).c_str());

      ret_int = regexec(&color_pattern, color, 0, NULL, 0);

      regfree(&color_pattern);

      if (!ret_int) {
        Serial.println("color input valid");
      }
      else if (ret_int == REG_NOMATCH) {
        Serial.println("Color input NOT valid");
        server.send(400, "text/plain", "Invalid Idle Color");
        return;
      }
      else {
        Serial.print("Regex Failed at Color");
        server.send(500, "text/plain", "Internal Server Error, Error 3");
        return;
      }
      
    }
 
    if (server.argName(i) == "enabled") {
      int length = strlen(server.arg(i).c_str());

      if (((strcmp((server.arg(i).c_str()), "true")) != 0) && ((strcmp((server.arg(i).c_str()), "false")) != 0)) {
        server.send(400, "text/plain", "Invalid value for field: enabled");
        return;
      }

      enabled = (server.arg(i).c_str());
    }
  }

  if (has_internet) {
    unsigned long sys_epoch_time =  timeClient.getEpochTime();

    Serial.printf("Cur Time: %ld\n", sys_epoch_time);
    Serial.printf("Inc Time: %ld\n", (long int)inc_time); 
    // cast to normal long to allow for negatives
    if (((long)(sys_epoch_time - inc_time)) > 15) { 
      server.send(400, "text/plain", "Took over 15 seconds to send, or you're sending fake requests.");
      return;
    }

    if (((long)(sys_epoch_time - inc_time)) < (-5)) { 
      server.send(400, "text/plain", "You sent this request over 5 seconds in the past, stop sending fake requests.");
      return;
    }
  }


  if (strcmp(inc_hash_str, "") == 0) {
    server.send(400, "text/plain", "Stop the fake requests.");
    return;
  }


  SHA256 hasher;
  
  //An attempt to deallocate some memory first.
  if (true) {
    Serial.printf("Adding time:   %s\n", inc_time_str);
    hasher.doUpdate(inc_time_str);

    Serial.printf("Adding color: %s\n", idle_color);
    hasher.doUpdate(idle_color);

    Serial.printf("Adding enb_d: %s\n", enabled);
    hasher.doUpdate(enabled);

    File verif_file = LittleFS.open("/verif.txt", "r");
    String verif_str = verif_file.readString();
    const char *verif_chars = verif_str.c_str();
    verif_file.close();

    Serial.printf("Adding verif:  %s\n", verif_chars);
    hasher.doUpdate(verif_chars);

    File pass_file = LittleFS.open("/pass.txt", "r");
    String pass_str = pass_file.readString();
    const char *pass_chars = pass_str.c_str();
    pass_file.close();

    Serial.println("Adding pass"); //  %s\n", pass_chars);
    hasher.doUpdate(pass_chars);
  }

  byte calc_hash[SHA256_SIZE];
  hasher.doFinal(calc_hash);


  //Code taken from Serial.print()
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *hash_str = &buf[sizeof(buf) - 1];

  *hash_str = '\0';

  // Code taken from Serial.print() and modified to work for my scenario.
  // Apparently the original doesn't fully work, probably edge case.
  for (int i = (SHA256_SIZE-1); i >= 0; i--) {
    unsigned long hash_long = (unsigned long)calc_hash[i];
    for (int j = 0; j <= 1; j++ ) {
      unsigned long m = hash_long;
      hash_long /= HEX;
      char c = m - HEX * hash_long;
      *--hash_str = (c < 10 ? (c + '0') : c + 'A' - 10);
    }
  }


  Serial.printf("Incoming Hash:   %s\n", inc_hash_str);
  Serial.printf("Calculated Hash: %s\n", hash_str);

  ESP.wdtFeed(); // Before watchdog bites me.

  if (strcmp(hash_str, inc_hash_str) == 0) {
    Serial.println("Password matches");
    server.send(200, "text/plain", "Password Good.");
  }
  else {
    Serial.println("Password do not match");
    server.send(401, "text/plain", "Wrong password.");
    return;
  }
  
  String to_write = "true ";
  to_write += enabled;
  to_write += " ";
  to_write += idle_color;

  const char *to_write_char = (to_write.c_str());
  Serial.printf("Writing to add time file: %s\n", to_write_char);
  File config_file = LittleFS.open("/config.txt", "w");
  config_file.write(to_write_char);
  config_file.close();

  changeIdle();
}


void readButton(void) {
  if(digitalRead(BUTTON_PIN) == LOW) {
    // TODO Reset stuff
    LittleFS.remove("/config.txt"); // Ensure the file is removed.
    LittleFS.remove("/pass.txt");
    LittleFS.remove("/wifi/user.txt");
    LittleFS.remove("/wifi/pass.txt");
    LittleFS.remove("/wifi/SSID.txt");
    LittleFS.remove("/wifi/hostname.txt");
    LittleFS.remove("/wifi/method.txt");
    File config_file = LittleFS.open("/config.txt", "w");
    const char *to_write = default_color.c_str();
    config_file.write("true true ");
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
  
  char *true_ptr = "true";

  // Check if init done.
  if (strcmp(config_values[0], true_ptr) == 0) {
    Serial.println("CV0: True");


    Serial.println("Generating Verification Data");
    generateVerification();

    server.on("/", handleLoginPage);
    server.on("/login.html", handleLoginPage);
    server.on("/js/verification.js", handleVerificationJS);
    server.on("/login", handleCredentialLogin);
    server.on("/js/login.js", handleLoginJS);
    server.on("/js/sjcl.js", handleSCJL);
    server.on("/hostname", handleHostname);
    server.on("/timer.html", handleTimerPage);
    server.on("/gettimer", handleGetTimer);
    server.on("/js/timer.js", handleTimerJS);
    server.on("/timer", handleSetTimer);
    server.on("/settings.html", handleSettingsPage);
    server.on("/js/settings.js", handleSettingsJS);
    server.on("/getsettings", handleGetSettings);
    server.on("/warn", handleSetWarning);
    server.on("/idle", handleSetIdle);
    server.on("/opcolors", handleSetOpColors);
  }
  else {
    Serial.println("CV0: False");
    // Init code.
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.println("Scanning Networks");
    delay(100); // Give it time to disconnect.
    WiFi.scanNetworksAsync(appendFoundNetworks, false);
    while (WiFi.scanComplete() == -1) {
      delay(250);
      Serial.print("."); // Wait for the scan to finish.
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

  changeIdle();

  Serial.println("Starting WiFi");
  initWifi();

  if (Ping.ping("http://1.1.1.1")) {
    timeClient.begin();
    has_internet = true;
  }

  server.on("/js/common.js", handleCommonJS);
  server.on("/css/common.css", handleCommonCSS);
  server.on("/favicon.ico", handleFavicon);

  server.begin();
}

int millis_track = 0;
unsigned long real_millis_check = 0;
unsigned long real_millis;
unsigned long corrected_millis;
boolean initialized = false;
boolean offset_enable = true;
unsigned long expected_epoch_time;
long slewed_offset;

void updateOffset(void) {
  if ((unsigned long)(real_millis - real_millis_check) >= 10000) {
    millis_offset = (timeClient.getEpochTime() - expected_epoch_time);
    expected_epoch_time += 10;
    real_millis_check = real_millis;

    if (((slewed_offset - (millis_offset * 1000)) > 30000) || ((slewed_offset - (millis_offset * 1000)) < -30000)) {
      Serial.printf("Current Broken Epoch time:  %d\n", timeClient.getEpochTime());
      timeClient.forceUpdate();
    }

    if (slewed_offset == (millis_offset * 1000)) {
      // do nothing.
    }
    else if (slewed_offset < (millis_offset * 1000)) {
      slewed_offset += 125;
    }
    else if (slewed_offset > (millis_offset * 1000)) {
      slewed_offset -= 125;
    }

    Serial.printf("%d %d\n", millis_track, millis_offset);
    Serial.printf("Current Epoch time:  %d\n", timeClient.getEpochTime());
    Serial.printf("Current millis time: %d\n", real_millis);
    Serial.printf("Corrected millis:    %d\n", corrected_millis);
  }
}

void loop(void) {
  real_millis = millis();
  corrected_millis = (millis() + slewed_offset);

  server.handleClient();

  if (add_time) {
    timerAdd();
    add_time = false;
  }

  if (has_internet) {
    timeClient.update();
    if (initialized){
      updateOffset();
    }
    else {
      expected_epoch_time = (timeClient.getEpochTime() + 10);
      real_millis_check = real_millis;
      initialized = true;
      Serial.printf("Init Epoch time:  %d\n", timeClient.getEpochTime());
      Serial.printf("Init millis time: %d\n", real_millis);
    }
  }

  ESP.wdtFeed(); // Watchdog is watching along with big brother.

  if (timer_on) {
    setTimer();
  }

  readButton();
}