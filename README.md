## LED Timer

###### C++, JS, HTML, and CSS code written to run on an ESP8266.

Designed to be run on custom hardware, though could be hooked up with a level shifter.



### Features

---

- ###### WiFi connected

  - Connect through LAN to an onboard web server.

- ###### Password protected

  - Requires login to set timer, prevents unauthorized access.

- ###### Add time to timer

- ###### Customizable color light when idle

- ###### Time slewing

  - Corrects the time using NTP time (requires internet access)
  - Adjusts the time by 125 ms every 10 sec if the time is off.
  - Internet check every 30 mins max.

- ###### Reset Button

  - Clears passwords by overwriting passwords with random data and deletes all other files.



#### Libraries in Use

---

##### FastLED

- Controls the LEDs

##### ESP8266Ping

- Determines if the device has internet access. Pings 1.1.1.1 to check.

##### ESP8266TrueRandom

- Generates random data for verification code which changes every boot.

##### NTPClient

- Note: Patched with [this](https://github.com/arduino-libraries/NTPClient/pull/43/commits/055b927b4d2bbdbef7ee473850ea9fae5f017c50) to prevent user offset on epoch time and the related parts of [this](https://gist.github.com/tobozo/8bcd16391025352dcf9c8ce66f8fdae0) to check for NTP packet validity.

- Connects to NTP server to get access to internet time.
- Used to slew time (by 1/8sec per 10 sec) if millis is either slow or fast.

