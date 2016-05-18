#include <esp8266_client.h>

SoftwareSerial swSerial(8,9);
Esp8266 esp8266(&swSerial);

Esp8266Client client;

byte server[] = { 64, 233, 187, 99 }; // Google

void setup()
{
  Serial.begin(115200);
  esp8266.begin();
  
  delay(1000);

  Serial.println("connecting...");

  if (client.connect(server, 80)) {
    Serial.println("connected");
    client.println("GET /search?q=arduino HTTP/1.0");
    client.println();
  } else {
    Serial.println("connection failed");
  }
}

void loop()
{
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnected.");
    client.stop();
    for(;;)
      ;
  }
}
