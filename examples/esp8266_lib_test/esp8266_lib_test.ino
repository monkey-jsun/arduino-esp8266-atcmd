#define ESP8266_SW_RX   9 
#define ESP8266_SW_TX   8 

#include "esp8266_lib.h"

SoftwareSerial swSerial(8,9);
Esp8266 esp8266(&swSerial);

const char mySSID[] = "xzy_123";
const char myPSK[] = "111222333444";

const char server[] = "example.com";
const char httpRequest[] = "GET / HTTP/1.1\r\n"
                           "Host: example.com\r\n"
                           "Connection: close\r\n\r\n";
const char httpRequest2[] = "GET /hello.html HTTP/1.1\r\n"
                           "Host: junsun.net\r\n"
                           "Connection: close\r\n\r\n";

#define TEST(x,y) do { \
    Serial.print(F("test ")); \
    Serial.print(#x); \
    Serial.print(F(" .... ")); \
    if (x y) Serial.println(F("pass")); \
    else { Serial.println(F("fail")); for(;;); } \
} while (0)

char buf[64];
void setup() {
  
  Serial.begin(115200);

  TEST(esp8266.begin(), >= 0);
  
  // raw_test();
  
  general_test();

  server_test();

  client_test();

  Serial.println(F("All done! Entering loop ..."));
}

void loop() {
  // Serial.println(F("We are in loop"));
  delay(2000);
}

void raw_test() {
  esp8266.rawTest("AT+CIPSTART=0,\"TCP\",\"example.com\",80,60", 5000);  
  Serial.println(strlen(httpRequest));
  esp8266.rawTest("AT+CIPSENDEX=0,56", 5000);
  esp8266.rawTest(httpRequest, 5000);
}

void general_test() {
  serialTrigger(F("Press any key to begin general test."));

  TEST(esp8266.getVersion(buf,NULL,NULL), >= 0);
  Serial.print(F("AT version is ")); Serial.println(buf);
  
  TEST(esp8266.getVersion(NULL, buf,NULL), >= 0);
  Serial.print(F("SDK version is ")); Serial.println(buf);
  
  TEST(esp8266.getVersion(NULL, NULL, buf), >= 0);
  Serial.print(F("Compile time is ")); Serial.println(buf);
  
  TEST(esp8266.connectAP(mySSID, myPSK), >0);

  TEST(esp8266.getAP(buf), >=0);
  Serial.print(F("getAP()=")); Serial.println(buf);
  TEST(strcmp(buf, mySSID), == 0);

  TEST(esp8266.getLocalMAC(buf), >=0);
  Serial.print(F("getLocalMAC()=")); Serial.println(buf);

  IPAddress ip;
  TEST(esp8266.getLocalIP(ip), >=0);
  Serial.print(F("getLocalIP()=")); Serial.println(ip);

  Serial.println(F("disconnecting AP ..."));
  TEST(esp8266.disconnectAP(), >=0);
    
  delay(5000);

  TEST(esp8266.getAP(buf), >=0);
  TEST(strcmp(buf, ""), == 0);
}

void server_test()
{
    serialTrigger(F("Press any key to begin TCP SERVER test."));

    TEST(esp8266.connectAP(mySSID, myPSK), >0);
    IPAddress ip;
    TEST(esp8266.getLocalIP(ip), >=0);
    Serial.print(F("getLocalIP()=")); Serial.println(ip);

    TEST(esp8266.tcpServerStart(80), >= 0);

    Serial.println(F("Open browser at server ... waiting for client"));
    for (;;) {
        if (esp8266.tcpConnected()) break;
    }
    Serial.println(F("client connected!"));
    
    // read data
    Serial.println(F("Now dump received data:"));
    while(!esp8266.tcpAvailable());
    while(esp8266.tcpAvailable())
      Serial.write(esp8266.tcpRead());
    Serial.println(F("\n===END OF TCP DATA===\n"));

    // send something back
    TEST(esp8266.tcpWrite("<h1>Hello, there</h1>"), >= 0);

    TEST(esp8266.tcpClose(), >=0);

    // chrome browser sends two requests
    while(!esp8266.tcpConnected());
    Serial.println(F("Now dump received data:"));
    while(!esp8266.tcpAvailable());
    while(esp8266.tcpAvailable())
      Serial.write(esp8266.tcpRead());
    Serial.println(F("\n===END OF TCP DATA===\n"));

    // send something back
    TEST(esp8266.tcpWrite("<h1>Hello, there</h1>"), >= 0);

    TEST(esp8266.tcpClose(), >=0);

    // stop server
    TEST(esp8266.tcpServerStop(), >= 0);

    /*
    Serial.println(F("Restart server and re-stop it ..."));
    TEST(esp8266.tcpServerStart(80), >= 0);
    TEST(esp8266.tcpServerStop(), >= 0);
    */
}

void client_test()
{
    serialTrigger(F("Press any key to begin TCP client test."));

    // TEST(esp8266.connectAP(mySSID, myPSK), >0);

    TEST(esp8266.tcpConnect(server, 80, 30000), >=0);

    while (!esp8266.tcpConnected()) ;
    TEST(esp8266.tcpWrite(httpRequest), >=0);

    Serial.println(F("Now dump received data:"));
    while(!esp8266.tcpAvailable());
    while(esp8266.tcpAvailable())
      Serial.write(esp8266.tcpRead());
    Serial.println(F("\n===END OF TCP DATA===\n"));

    // server disconnects
    delay(1000);
    TEST(esp8266.tcpConnected(), ==false);

    // now open a second session
    TEST(esp8266.tcpConnect("junsun.net", 80, 30000), >=0);
    while (!esp8266.tcpConnected()) ;
    TEST(esp8266.tcpWrite(httpRequest2), >=0);

    Serial.println(F("Now dump received data:"));
    while(!esp8266.tcpAvailable());
    while(esp8266.tcpAvailable())
      Serial.write(esp8266.tcpRead());
    Serial.println(F("\n===END OF TCP DATA===\n"));

    // server disconnects
    delay(1000);
    TEST(esp8266.tcpConnected(), ==false);
}

void serialTrigger(String message)
{
  Serial.println();
  Serial.println(message);
  Serial.println();
  while (!Serial.available())
    ;
  while (Serial.available()) {
    Serial.read();
    delay(2);  // deal with slow serial case, 2ms is good to cover 9600
  }
}
