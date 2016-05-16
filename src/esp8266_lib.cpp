/******************************************************************************
******************************************************************************/

#include <Arduino.h>

#include "esp8266_const.h"
#include "esp8266_debug.h"
#include "esp8266_lib.h"


////////////////////////
// Buffer Definitions //
////////////////////////
#define ESP8266_RX_BUFFER_LEN 128 // Number of bytes in the serial receive buffer
char esp8266RxBuffer[ESP8266_RX_BUFFER_LEN];
unsigned int bufferHead; // Holds position of latest byte placed in buffer.

////////////////////
// Initialization //
////////////////////

Esp8266::Esp8266(SoftwareSerial *swSerial)
{
	_serial = swSerial;
}

int16_t Esp8266::begin(unsigned long baudRate)
{
	_serial->begin(baudRate);

	if (test() < 0) return ESP8266_RSP_FAIL;
	if (setMux(1) < 0)	return ESP8266_RSP_FAIL;
	if (echo(false) < 0) return ESP8266_RSP_FAIL;

	return ESP8266_RSP_SUCCESS;
}

///////////////////////
// Basic AT Commands //
///////////////////////

int16_t Esp8266::test()
{
	sendCommand(ESP8266_TEST); // Send AT

	return readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
}

int16_t Esp8266::echo(bool enable)
{
	if (enable)
		sendCommand(ESP8266_ECHO_ENABLE);
	else
		sendCommand(ESP8266_ECHO_DISABLE);
	
	return readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
}

int16_t Esp8266::getVersion(char * ATversion, char * SDKversion, char * compileTime)
{
	sendCommand(ESP8266_VERSION); // Send AT+GMR
	// Example Response: AT version:0.30.0.0(Jul  3 2015 19:35:49)\r\n (43 chars)
	//                   SDK version:1.2.0\r\n (19 chars)
	//                   compile time:Jul  7 2015 18:34:26\r\n (36 chars)
	//                   OK\r\n
	// (~101 characters)
	// Look for "OK":
	int16_t rsp = (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT) > 0);
	if (rsp > 0)
	{
		char *p, *q;
		// Look for "AT version" in the rxBuffer
		p = strstr(esp8266RxBuffer, "AT version:");
		if (p == NULL) return ESP8266_RSP_UNKNOWN;
		p += strlen("AT version:");
		q = strchr(p, '\r'); // Look for \r
		if (q == NULL) return ESP8266_RSP_UNKNOWN;
		if (ATversion) {
			strncpy(ATversion, p, q-p);
			ATversion[q-p]=0;
		}
		
		// Look for "SDK version:" in the rxBuffer
		p = strstr(esp8266RxBuffer, "SDK version:");
		if (p == NULL) return ESP8266_RSP_UNKNOWN;
		p += strlen("SDK version:");
		q = strchr(p, '\r'); // Look for \r
		if (q == NULL) return ESP8266_RSP_UNKNOWN;
		if (SDKversion) {
			strncpy(SDKversion, p, q-p);
			SDKversion[q-p]=0;
		}
		
		// Look for "compile time:" in the rxBuffer
		p = strstr(esp8266RxBuffer, "compile time:");
		if (p == NULL) return ESP8266_RSP_UNKNOWN;
		p += strlen("compile time:");
		q = strchr(p, '\r'); // Look for \r
		if (q == NULL) return ESP8266_RSP_UNKNOWN;
		if (compileTime) {
			strncpy(compileTime, p, q-p);
			compileTime[q-p]=0;
		}
	}
	
	return rsp;
}

////////////////////
// WiFi Functions //
////////////////////

int16_t Esp8266::connectAP(const char * ssid)
{
	connectAP(ssid, "");
}

// connect()
// Input: ssid and pwd const char's
// Output:
//    - Success: >0
//    - Fail: <0 (esp8266_cmd_rsp)
int16_t Esp8266::connectAP(const char * ssid, const char * pwd)
{
	// Send : AT+CWJAP="ssid","pwd"
	clearBuffer();
	if (pwd)
		sprintf(esp8266RxBuffer,"\"%s\",\"%s\"",ssid,pwd);
	else
		sprintf(esp8266RxBuffer,"\"%s\"",ssid);
	
	sendCommand(ESP8266_CONNECT_AP,ESP8266_CMD_SETUP,esp8266RxBuffer);
	
	return readForResponses(RESPONSE_OK, RESPONSE_FAIL, WIFI_CONNECT_TIMEOUT);
}

int16_t Esp8266::getAP(char * ssid)
{
	sendCommand(ESP8266_CONNECT_AP, ESP8266_CMD_QUERY); // Send "AT+CWJAP?"
	
	int16_t rsp = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	// Example Responses: No AP\r\n\r\nOK\r\n
	// - or -
	// +CWJAP:"WiFiSSID","00:aa:bb:cc:dd:ee",6,-45\r\n\r\nOK\r\n
	if (rsp > 0)
	{
		// Look for "No AP"
		if (strstr(esp8266RxBuffer, "No AP") != NULL) {
			ssid[0]=0;
			return ESP8266_RSP_SUCCESS;
		}
		
		// Look for "+CWJAP"
		char * p = strstr(esp8266RxBuffer, ESP8266_CONNECT_AP);
		if (p != NULL)
		{
			p += strlen(ESP8266_CONNECT_AP) + 2;
			char * q = strchr(p, '"');
			if (q == NULL) return ESP8266_RSP_UNKNOWN;
			strncpy(ssid, p, q-p);
			ssid[q-p] = 0; 	// terminate string
			return ESP8266_RSP_SUCCESS;
		}
	}
	
	return rsp;
}

int16_t Esp8266::disconnectAP()
{
	sendCommand(ESP8266_DISCONNECT); // Send AT+CWQAP
	// Example response: \r\n\r\nOK\r\nWIFI DISCONNECT\r\n
	// "WIFI DISCONNECT" comes up to 500ms _after_ OK. 
	return readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
}

// localIP()
// Input: none
// Output:
//    - Success: Device's local IPAddress
//    - Fail: 0
int16_t Esp8266::getLocalIP(IPAddress &returnIP)
{
	sendCommand(ESP8266_GET_LOCAL_IP); // Send AT+CIFSR\r\n
	// Example Response: +CIFSR:STAIP,"192.168.0.114"\r\n
	//                   +CIFSR:STAMAC,"18:fe:34:9d:b7:d9"\r\n
	//                   \r\n
	//                   OK\r\n
	// Look for the OK:
	int16_t rsp = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if (rsp <= 0) return rsp;

	// Look for "STAIP" in the rxBuffer
	char * p = strstr(esp8266RxBuffer, "STAIP");
	if (p != NULL)
	{
		p += 7; // Move p seven places. (skip STAIP,")
		for (uint8_t i = 0; i < 4; i++)
		{
			char tempOctet[4];
			memset(tempOctet, 0, 4); // Clear tempOctet
			
			size_t octetLength = strspn(p, "0123456789"); // Find length of numerical string:
			if (octetLength >= 4) // If it's too big, return an error
				return ESP8266_RSP_UNKNOWN;
			
			strncpy(tempOctet, p, octetLength); // Copy string to temp char array:
			returnIP[i] = atoi(tempOctet); // Move the temp char into IP Address octet
			
			p += (octetLength + 1); // Increment p to next octet
		}
	}
	
	return ESP8266_RSP_SUCCESS;
}

int16_t Esp8266::getLocalMAC(char * mac)
{
	sendCommand(ESP8266_GET_STA_MAC, ESP8266_CMD_QUERY); // Send "AT+CIPSTAMAC?"

	int16_t rsp = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);

	if (rsp > 0)
	{
		// Look for "+CIPSTAMAC"
		char * p = strstr(esp8266RxBuffer, ESP8266_GET_STA_MAC);
		if (p != NULL)
		{
			p += strlen(ESP8266_GET_STA_MAC) + 2;
			char * q = strchr(p, '"');
			if (q == NULL) return ESP8266_RSP_UNKNOWN;
			strncpy(mac, p, q - p); // Copy string to temp char array:
			mac[q-p]=0;
			return 1;
		}
	}

	return rsp;
}

/////////////////////
// TCP/IP Commands //
/////////////////////

int16_t Esp8266::tcpConnect(const char * destination, uint16_t port, uint16_t keepAlive)
{
	ASSERT(_tcpState == ESP8266_TCP_NONE);
	
	if (tcpConnected()) return ESP8266_RSP_FAIL;
	
	// Send : AT+CIPSTART="TCP","192.168.101.110",1000
	clearBuffer();
	sprintf(esp8266RxBuffer,"%d,\"TCP\",\"%s\",%d,%d", 0, destination, port, keepAlive/500);
	sendCommand(ESP8266_TCP_CONNECT, ESP8266_CMD_SETUP, esp8266RxBuffer);
		
	// Example good: CONNECT\r\n\r\nOK\r\n
	// Example bad: DNS Fail\r\n\r\nERROR\r\n
	// Example meh: ALREADY CONNECTED\r\n\r\nERROR\r\n
	int16_t result = readForResponses(RESPONSE_OK, RESPONSE_ERROR, CLIENT_CONNECT_TIMEOUT);
	if (result >= 0) 
		_tcpState = ESP8266_TCP_CLIENT;
	
	return result;
}

bool Esp8266::tcpConnected()
{
	readForAsync(0);
	return _tcpConnected;
}

int16_t Esp8266::tcpWrite(const char * msg)
{
	return tcpWrite((const uint8_t*)msg, strlen(msg));
}

int16_t Esp8266::tcpWrite(const uint8_t *buf, size_t size)
{
	if (size > 2048)
		return ESP8266_CMD_BAD;
	char params[8];
	sprintf(params, "%d,%d", 0, size);
	sendCommand(ESP8266_TCP_SEND, ESP8266_CMD_SETUP, params);
	
	int16_t rsp = readForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_RESPONSE_TIMEOUT);
	if (rsp > 0)
	{
		write(buf, size);
		rsp = readForResponse("SEND OK", COMMAND_RESPONSE_TIMEOUT);
		
		if (rsp > 0)
			return size;
	}
	
	return rsp;
}

bool Esp8266::tcpAvailable()
{
	ASSERT(_tcpConnected);
	readForAsync(0);
	return _tcpDataSize > 0;
}

uint8_t Esp8266::tcpRead()
{
	ASSERT(_tcpDataSize > 0);
	while (!_serial->available());
	_tcpDataSize--;
	return _serial->read();
}

int16_t Esp8266::tcpRead(uint8_t *buf, size_t size) 
{
	ASSERT(_tcpDataSize > 0);
	size_t s = min(_tcpDataSize, size);
	for (int i=0; i< s; i++)
		buf[i] = tcpRead();
}

int16_t Esp8266::tcpClose()
{
	char params[2];
	sprintf(params, "%d", 0);
	sendCommand(ESP8266_TCP_CLOSE, ESP8266_CMD_SETUP, params);
	
	return readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
}

int16_t Esp8266::setMux(uint8_t mux)
{
	char params[2] = {0, 0};
	params[0] = (mux > 0) ? '1' : '0';
	sendCommand(ESP8266_TCP_MULTIPLE, ESP8266_CMD_SETUP, params);
	
	return readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
}

int16_t Esp8266::tcpServerStart(uint16_t port)
{
	ASSERT(_tcpState == ESP8266_TCP_NONE);

	char params[10];	
	sprintf(params, "1,%d", port);
	sendCommand(ESP8266_SERVER_CONFIG, ESP8266_CMD_SETUP, params);	
	int16_t ret = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if (ret >= 0) _tcpState = ESP8266_TCP_SERVER;
	
	return ret;
}

int16_t Esp8266::tcpServerStop()
{
	ASSERT(_tcpState == ESP8266_TCP_SERVER);
	
	char params[]="0";
	sendCommand(ESP8266_SERVER_CONFIG, ESP8266_CMD_SETUP, params);
	int16_t ret = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if (ret >= 0) _tcpState = ESP8266_TCP_NONE;
	return ret;
}

int16_t Esp8266::ping(IPAddress ip)
{
	char ipStr[17];
	sprintf(ipStr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	return ping(ipStr);
}

int16_t Esp8266::ping(char * server)
{
	char params[strlen(server) + 3];
	sprintf(params, "\"%s\"", server);
	// Send AT+Ping=<server>
	sendCommand(ESP8266_PING, ESP8266_CMD_SETUP, params); 
	// Example responses:
	//  * Good response: +12\r\n\r\nOK\r\n
	//  * Timeout response: +timeout\r\n\r\nERROR\r\n
	//  * Error response (unreachable): ERROR\r\n\r\n
	int16_t rsp = readForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_PING_TIMEOUT);
	if (rsp > 0)
	{
		char * p = searchBuffer("+");
		p += 1; // Move p forward 1 space
		char * q = strchr(p, '\r'); // Find the first \r
		if (q == NULL)
			return ESP8266_RSP_UNKNOWN;
		char tempRsp[10];
		strncpy(tempRsp, p, q - p);
		return atoi(tempRsp);
	}
	else
	{
		if (searchBuffer("timeout") != NULL)
			return 0;
	}
	
	return rsp;
}

//////////////////////////////////////////////////
// Private, Low-Level, Ugly, Hardware Functions //
//////////////////////////////////////////////////

size_t Esp8266::write(const uint8_t* buf, size_t size)
{
	return _serial->write(buf, size);
}

void Esp8266::sendCommand(const char * cmd, enum esp8266_command_type type, const char * params)
{
	drainTcpData();	// we should not get into this situation often!
	
	_serial->print(F("AT"));
	_serial->print(cmd);
	DEBUG_VERBOSE(Serial.print(F("AT")));
	DEBUG_VERBOSE(Serial.print(cmd));
	if (type == ESP8266_CMD_QUERY) {
		_serial->print(F("?"));
		DEBUG_VERBOSE(Serial.print(F("?")));
	} 
	else if (type == ESP8266_CMD_SETUP)
	{
		_serial->print(F("="));
		_serial->print(params);
		DEBUG_VERBOSE(Serial.print(F("=")));
		DEBUG_VERBOSE(Serial.print(params));
	}
	_serial->print(F("\r\n"));
	DEBUG_VERBOSE(Serial.print(F("\r\n")));
}

int16_t Esp8266::readForAsync(unsigned int timeout)
{
	// don't check for async msg if we still have tcp data
	if (_tcpDataSize > 0 ) 
		return ESP8266_RSP_SUCCESS;
	else
		return readForResponses(NULL, NULL, timeout);
}

int16_t Esp8266::readForResponse(const char * rsp, unsigned int timeout)
{
	return readForResponses(rsp, NULL, timeout);
}

/*
  read sufficient chars and do async msg processing.
  "sufficient chars" 
	- drain all avaialbe chars on remote
		- wait for maximum 2ms gap
	- until we have a hit or 2ms gap timeout
*/
int16_t Esp8266::readForResponses(const char * pass, const char * fail, unsigned int timeout)
{
	ASSERT(_tcpDataSize == 0);
	clearBuffer();	// Clear the class receive buffer (esp8266RxBuffer)

	// we persistent-read next char if available and scan for keywords
	// if we run out of input, we check for timeout (outer loop)
	unsigned long timeIn = millis();	// Timestamp coming into function
	do {
		for(;;) {
			if (! readByteToBuffer()) break;
			// if one of them is not NULL, we are in command mode; discard tcp data
			checkAsyncMsg(pass || fail);	
			if (pass)
				if (searchBuffer(pass))	// Search the buffer for goodRsp
					return bufferHead;	// Return how number of chars read
			if (fail)
				if (searchBuffer(fail))
					return ESP8266_RSP_FAIL;
		}
	} while (millis() < timeIn + timeout); // While we haven't timed out
	// Serial.println(F("\n==timeout==\n")); // jsun
	
	if (bufferHead > 0) // If we received any characters
		return ESP8266_RSP_UNKNOWN; // Return unkown response error code
	else // If we haven't received any characters
		return ESP8266_RSP_TIMEOUT; // Return the timeout error code
}

bool Esp8266::checkAsyncMsg(bool discardTcpData)
{
	bool flag=false;
	
	if (searchBuffer(RESPONSE_CONNECT_0)) {
		DEBUG_VERBOSE(Serial.println(F("\ntcp connected!")));
		flag=true;
		_tcpConnected=true;
	} else if (searchBuffer(RESPONSE_CONNECT)) {
		flag=true;
		// this should only happen in server mode
		Serial.println(F("TODO: non-0 connection detected"));
		// Serial.println(esp8266RxBuffer);
		// ASSERT(false);
	}
	
	// we don't care about non-zero channel closing
	if (searchBuffer(RESPONSE_CLOSED_0)) {
		DEBUG_VERBOSE(Serial.println(F("\ntcp disconnected!")));
		flag=true;
		_tcpConnected=false;
		// multiple possibilities to get here:
		// - we actively kill server
		// - server's client disconnect
		// - we are client and server disconnect
		// - we actively called tcpClose()
		if (_tcpState == ESP8266_TCP_CLIENT)	// quit client mode when session ends
			_tcpState = ESP8266_TCP_NONE;		
	}
	
	// we have tcp data to read
	// example
	// 
	// +IPD,0,357:GET /favicon.ico HTTP/1.1
	// Host: 10.10.1.193
	if (searchBuffer(RESPONSE_IPD)) {
		ASSERT(_tcpDataSize == 0);
		flag=true;
		
		// read until we have ":"
		VERIFY(readByteToBuffer(), == true); 	// we must read this byte
		char c = '0';
		VERIFY(readByteToBuffer(), == true);
		uint16_t currPos = bufferHead;
		do {
			VERIFY(readByteToBuffer(), == true);
		} while (bufferTail() != ':');
		_tcpDataSize = atoi(esp8266RxBuffer+currPos);
		
		// if we are waiting for command response we should discard tcp data (WARNING)
		if (discardTcpData || c != '0') {
			drainTcpData();
		} else {
			DEBUG_VERBOSE(Serial.print(F("IPD size:")));
			DEBUG_VERBOSE(Serial.println(_tcpDataSize));
		}
	}
	
	if (flag) {
		clearBuffer();
	}
	return flag;
}
 
void Esp8266::drainTcpData()
{
	if (_tcpDataSize == 0) return;
	
	Serial.print(F("\nWARNING : discarding TCP data (bytes) : "));
	Serial.println(_tcpDataSize);
	
	for (; _tcpDataSize > 0; _tcpDataSize--) {
		ASSERT(!_serial->overflow());
		while (! _serial->available());
		uint8_t c = _serial->read();
		DEBUG_VERBOSE(Serial.write(c));
	}
}

void Esp8266::drainAllData()
{
	drainTcpData();
	
	for (;;) {
		ASSERT(!_serial->overflow());
		if (!_serial->available()) delay(2);
		if (!_serial->available()) return;
		uint8_t c = _serial->read();
		DEBUG_VERBOSE(Serial.write(c));
	}	
}

//////////////////
// Buffer Stuff //
//////////////////
uint8_t Esp8266::bufferTail() {
	if (bufferHead==0)
		return 0;
	else
		return esp8266RxBuffer[bufferHead-1];
}

void Esp8266::clearBuffer()
{
	bufferHead = 0;
}	

bool Esp8266::readByteToBuffer()
{
	if (_tcpDataSize > 0)		// we only read cmd data
		return false;

	WARN(!_serial->overflow());
	
	// Read a byte in; extra is to make sure
	// aggressive continuous reading will choke on slow UART
	// This is important for reliable serial input processing
	// 2ms delay is good enough to cover 9600 speed
	// in theory 1ms should be sufficient, but in practice we have
	// seen it is not good enough
	if (!_serial->available()) delay(2);
	
	if (!_serial->available()) return false;
	
	char c = _serial->read();
	DEBUG_VERBOSE(Serial.write(c));
	
	// Store the data in the buffer
	esp8266RxBuffer[bufferHead++] = c;
	ASSERT(bufferHead < ESP8266_RX_BUFFER_LEN);
	esp8266RxBuffer[bufferHead] = 0;
	return true;
}

char * Esp8266::searchBuffer(const char * test)
{
	return strstr((const char *)esp8266RxBuffer, test);
}

void Esp8266::rawTest(const char *cmd, uint16_t timeout)
{
	_serial->println(cmd);
	unsigned long timeIn = millis();	// Timestamp coming into function
	do {
		if (_serial->available())
			Serial.write(_serial->read());
	} while (millis() < timeIn + timeout);
}

