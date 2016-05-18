/******************************************************************************
******************************************************************************/

#ifndef __esp8266_client_h__
#define __esp8266_client_h__

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <IPAddress.h>
#include "Client.h"

#include "esp8266_lib.h"

class Esp8266Client : public Client {
	
public:
	Esp8266Client();

	virtual int connect(IPAddress ip, uint16_t port);
	virtual int connect(const char *host, uint16_t port);

	int connect(IPAddress ip, uint16_t port, uint32_t keepAlive);
	int connect(const char *host, uint16_t port, uint32_t keepAlive);

	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buf, size_t size);

	virtual int available();
	virtual int read();
	virtual int read(uint8_t *buf, size_t size);
	virtual int peek();
	virtual void flush();
	virtual void stop();
	virtual uint8_t connected();
	virtual operator bool();
};

#endif /* __esp8266_client_h__ */
