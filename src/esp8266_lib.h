/******************************************************************************
SparkFunESP8266WiFi.h
ESP8266 WiFi Shield Library Main Header File
Jim Lindblom @ SparkFun Electronics
Original Creation Date: June 20, 2015
http://github.com/sparkfun/SparkFun_ESP8266_AT_Arduino_Library


TCP states:
		- by default TCP is not up (TCP_NONE)
		- at this point we can start either a server (tcpServerStart()) or a client(tcpConnect())
		- one cannot start server if TCP is in client mode; vice versa;
		- when in TCP_SERVER state, one calls tcpServerStop() to tear it down
		- when in TCP_CLIENT state, TCP goes back to TCP_NONE when connection is down 
		  either disconnected by server or actively cancelled by calling tcpClose()
		- when connected, use tcpReceive()/tcpSend() to read/write with remote machine
		
	Separate convenience classes are provided to provide server or client functionalities
	that are compatible with other Arduino networking libraries

ESP8266 has 3 kinds of async input over serial that are interesting to us:

 1. 0,CONNECT - only happens in TCP_SERVER mode. A new client joins.
    A variation is 1,CONNECT (or 2,CONNECT) where we need to forcefully disconnect
 2. 0,CLOSE - TCP session terminates; both server mode and client mode
 3. +IPD,.... - input data; both server and client modes; when connected.

There two cases we need to check for aysnc input:
  1. after sending commands and checking for cmd responses
  2. check for TCP data or client connection.
  
For #2, we use readForAsync().  For #1, we have readForResponse() and readForResponses()

******************************************************************************/

#ifndef __esp8266_lib_h__
#define __esp8266_lib_h__

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <IPAddress.h>


///////////////////////////////
// Command Response Timeouts //
///////////////////////////////
#define COMMAND_RESPONSE_TIMEOUT 1000
#define COMMAND_PING_TIMEOUT 3000
#define WIFI_CONNECT_TIMEOUT 30000
#define COMMAND_RESET_TIMEOUT 5000
#define CLIENT_CONNECT_TIMEOUT 5000

#define ESP8266_MAX_SOCK_NUM 5
#define ESP8266_SOCK_NOT_AVAIL 255

enum esp8266_cmd_rsp {
	ESP8266_CMD_BAD = -5,
	ESP8266_RSP_MEMORY_ERR = -4,
	ESP8266_RSP_FAIL = -3,
	ESP8266_RSP_UNKNOWN = -2,
	ESP8266_RSP_TIMEOUT = -1,
	ESP8266_RSP_SUCCESS = 0
};

enum esp8266_wifi_mode {
	ESP8266_MODE_STA = 1,
	ESP8266_MODE_AP = 2,
	ESP8266_MODE_STAAP = 3
};

enum esp8266_command_type {
	ESP8266_CMD_QUERY,
	ESP8266_CMD_SETUP,
	ESP8266_CMD_EXECUTE
};

enum esp8266_encryption {
	ESP8266_ECN_OPEN,
	ESP8266_ECN_WPA_PSK,
	ESP8266_ECN_WPA2_PSK,
	ESP8266_ECN_WPA_WPA2_PSK
};

enum esp8266_connect_status {
	ESP8266_STATUS_GOTIP = 2,
	ESP8266_STATUS_CONNECTED = 3,
	ESP8266_STATUS_DISCONNECTED = 4,
	ESP8266_STATUS_NOWIFI = 5	
};

// current state of TCP connection
enum esp8266_tcp_state {
	ESP8266_TCP_NONE,
	ESP8266_TCP_SERVER,
	ESP8266_TCP_CLIENT
};

// general rules about return value
//  == 0: successful
//  >0 : sucessful with N chars read in buffer
//  <0 : error defined in ESP8266_CMD_RSP
class Esp8266
{
public:
	Esp8266(SoftwareSerial* swSerial);
	
	int16_t begin(unsigned long baudRate = 9600);
	
	///////////////////////
	// Basic AT Commands //
	///////////////////////
	int16_t getVersion(char * ATversion, char * SDKversion, char * compileTime);
	
	////////////////////
	// WiFi Functions //
	////////////////////
	int16_t connectAP(const char * ssid);
	int16_t connectAP(const char * ssid, const char * pwd);
	int16_t getAP(char * ssid);
	int16_t getLocalMAC(char * mac);
	int16_t getLocalIP(IPAddress& ip);
	int16_t disconnectAP();
	
	/*
	  TCP stuff
	*/
	int16_t tcpServerStart(uint16_t port);
	int16_t tcpServerStop();
	int16_t tcpConnect(const char * destination, uint16_t port, uint16_t keepAlive);
	int16_t tcpClose();
	bool tcpConnected();	// for both server and client connection

	int16_t tcpWrite(const char* msg);
	int16_t tcpWrite(const uint8_t *buf, size_t size);
	int16_t tcpRead(uint8_t *buf, size_t size);  // return size received; no waiting; <0 indicates error
	uint8_t tcpRead();
	int tcpPeek();		// return -1 is none is available
	int tcpAvailable();	// tcp data available for read in bytes
	
	int16_t ping(IPAddress ip);
	int16_t ping(char * server);
	
	void rawTest(const char* cmd, uint16_t timeout_ms);	// send cmd over serial and display response for timeout_ms ms

private:
	
	// helper commands
	int16_t test();
	int16_t setMux(uint8_t mux);
	int16_t echo(bool enable);

	// low-level send/receive
	void sendCommand(const char * cmd, enum esp8266_command_type type = ESP8266_CMD_EXECUTE, const char * params = NULL);
	int16_t readForResponse(const char * rsp, unsigned int timeout);
	int16_t readForResponses(const char * pass, const char * fail, unsigned int timeout);
	int16_t readForAsync(unsigned int timeout);
	bool checkAsyncMsg(bool discardTcpData);
	void drainTcpData();		// discard unread TCP data
	void drainAllData();
	
	size_t write(const uint8_t * buf, size_t size);
	
	/// clearBuffer() - Reset buffer pointer, set all values to 0
	void clearBuffer();
	
	// readByteToBuffer() - Read first byte from UART receive buffer
	// and store it in rxBuffer. Delay 1ms for persistent reliable read.
	// return false, if none avaiable to raed.
	bool readByteToBuffer();
	
	/// searchBuffer([test]) - Search buffer for string [test]
	/// Success: Returns pointer to beginning of string
	/// Fail: returns NULL
	char * searchBuffer(const char * test);
	
	// return last byte in rx buffer; return 0 otherwise
	uint8_t bufferTail();
	
	// esp8266 states
	SoftwareSerial * _serial;
	esp8266_tcp_state _tcpState=ESP8266_TCP_NONE;
	bool _tcpConnected=false;
	uint16_t _tcpServerPort;
	uint16_t _tcpDataSize=0;	// 0: no tcp data to read
};

extern Esp8266 esp8266;

#endif /* __esp8266_lib_h__ */
