# arduino-esp8266-atcmd
Library for Arduino board talking to an ESP8266 Wi-Fi board (e.g., Adafruit HUZZAH or Sparkfun ESP8266 shield) using AT command sets over serial UART port.
This is NOT a wifi library for Arduino running natively on an ESP8266 board.

Much of this library is derived from [*Sparkfun esp8266 library*](https://github.com/sparkfun/SparkFun_ESP8266_AT_Arduino_Library).  However, the change deviates so much that it becomes impossible to merge back.  So instead I created a new library.

In the rest of this file, I'll assume Arduino UNO board and HUZZAH breakout board.

# Hardware connection

Note you need to prepare HUZZAH board to run the original AT command firmware, not the LUA firmware from Adafruit.  See below for instructions on how to do that.

Below is a typical setup: 

* Connect esp8266 ground to Arduino ground
* Connect esp8266 V+ to 5V on Arduino board
* Connect esp8266 RX/TX pins to any two digital pins on Arduino except 0,1

http://junsun.net/local/esp8266-huzzah/arduino-esp8266-connection.png

# Flashing ESP8266 AT firmware

(TODO)


