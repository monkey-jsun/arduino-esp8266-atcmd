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

![Arduino-ESP8266 wiring](http://junsun.net/local/esp8266-huzzah/arduino-esp8266-connection.png)

# Prepare Adafruit HUZZAH board

## Soldering the board

For Arduino connections, we only need to use the FTDI pins at the end.  And we solder them on the back side of board, which is different from what Adafruit recommends.  See pictures below.
![Soldering pins - front](http://junsun.net/local/esp8266-huzzah/soldering-front.jpg)
![Soldering pins - back](http://junsun.net/local/esp8266-huzzah/soldering-back.jpg)

## Download software

Adafruit HUZZAH board comes with LUA firmware.  This library needs the original AT command firmware from the chip maker.

* dowload flash tool and manul
  * http://bbs.espressif.com/viewtopic.php?f=7&t=641
  * my version is [FLASH_DOWNLOAD_TOOLS_v2.4_150924.rar] (http://bbs.espressif.com/download/file.php?id=856) 
* download SDK which has firmware; download the non-OS version
  * http://bbs.espressif.com/viewtopic.php?f=46&t=850
  * my version is ESP8266_NONOS_SDK_V1.5.3_16_04_18

## Flashing ESP8266 AT firmware

* Connect HUZZAH board to PC via [FTDI cable.] (https://www.adafruit.com/product/70)
  * Note the black cable (marked with an arrow) connects to the ground pin on HUZZAH
* start and configure flash tool as shown.  
  * Note the serial port number might be different on your computer.  A quick way to find is actually use arduino IDE, "Tools"/"Ports".
![HUZZAH flashing config](http://junsun.net/local/esp8266-huzzah/flashing-config.png)
* Make HUZZAH into firmware downloading mode
  * press and hold GPIO0 pin
  * press and release Reset pin
  * release GPIO0 pin
  * a red LED should be lit after this sequence
* click "Start" button on the flashing tool to update firmware
* verify newly flashed AT firmware
  * reset HUZZAH board
  * start serial terminal program (e.g., termite) or the serial console from Arduino IDE
    * Note the windows built-in hyperterminal does not work
  * use baud rate 115200
  * type "AT" and should get "OK"
  * type "AT" and should get version numbers
  * below are my version numbers:
```
at+GMR
AT version:1.0.0.0(Apr 16 2016 13:02:45)
SDK version:1.5.3(aec24ac9)
compile time:Apr 18 2016 14:20:15
OK
```

## Setup ESP8266 for running
* switch to 9600 baud rate (to reduce software serial buffer overflow)
```
AT+UART_DEF=9600,8,1,0,0
```
* set to station mode (most common, default is SoftAP)
```
AT+CWMODE=1
```
* enable DHCP in station mode
```
AT+CWDHCP_DEF=1,1
```
* set AP and password
```
AT+CWJAP_DEF="tzuming_guest","12345678"
```
* enable auto-connect to AP on startup
```
AT+CWAUTOCONN=1
```
Now your HUZZAH board is ready for running with this library!
