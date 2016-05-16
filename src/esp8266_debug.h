#ifndef __esp8266_debug_h__
#define __esp8266_debug_h__

#define ESP8266_DEBUG
//#define ESP8266_DEBUG_VERBOSE

#ifdef ESP8266_DEBUG

	#define ASSERT(x) if (!(x)) { \
		Serial.print(F("assert failed at ")); \
		Serial.print(__func__); \
		Serial.print(F("()@")); \
		Serial.println(__LINE__); \
		for(;;); }
	#define WARN(x) if (!(x)) { \
		Serial.print(F("warning at ")); \
		Serial.print(__func__); \
		Serial.print(F("()@")); \
		Serial.println(__LINE__);}
	#define VERIFY(x, y) ASSERT(x y)
	#define DEBUG(x)  do { x; } while (0)
	#ifdef ESP8266_DEBUG_VERBOSE
		#define DEBUG_VERBOSE(x) DEBUG(x)
	#else
		#define DEBUG_VERBOSE(x)
	#endif

#else

	#define ASSERT(x)
	#define WARN(x)
	#define VERIFY(x, y) x
	#define DEBUG(x)
	#define DEBUG_VERBOSE(x)

#endif

#endif  /* __esp8266_debug_h__ */
