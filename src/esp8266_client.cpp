/******************************************************************************
******************************************************************************/

#include <Arduino.h>
#include "esp8266_client.h"
#include "esp8266_lib.h"

Esp8266Client::Esp8266Client()
{
}

	
int Esp8266Client::connect(IPAddress ip, uint16_t port)
{
	return connect(ip, port, 0);
}

int Esp8266Client::connect(const char *host, uint16_t port)
{
	return connect(host, port, 0);
}

int Esp8266Client::connect(IPAddress ip, uint16_t port, uint32_t keepAlive) 
{
	char ipAddress[16];
	sprintf(ipAddress, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	
	return connect((const char *)ipAddress, port, keepAlive);
}
	
int Esp8266Client::connect(const char* host, uint16_t port, uint32_t keepAlive) 
{
	// TODO: server-bound client cannot call this
	return esp8266.tcpConnect(host, port, keepAlive);
}

size_t Esp8266Client::write(uint8_t c)
{
	return write(&c, 1);
}

size_t Esp8266Client::write(const uint8_t *buf, size_t size)
{
	return esp8266.tcpWrite(buf, size);
}

int Esp8266Client::available()
{
	return esp8266.tcpAvailable();
}

int Esp8266Client::read()
{
	return esp8266.tcpRead();
}

int Esp8266Client::read(uint8_t *buf, size_t size)
{
	return esp8266.tcpRead(buf, size);
}

int Esp8266Client::peek()
{
	return esp8266.tcpPeek();
}

void Esp8266Client::flush()
{
	// nothing to be done
	// when we send tcp data, we always flush sw serial and wait for
	// positive response
}

void Esp8266Client::stop()
{
	esp8266.tcpClose();
}

uint8_t Esp8266Client::connected()
{
	return esp8266.tcpConnected();
}

Esp8266Client::operator bool()
{
	return connected();
}
