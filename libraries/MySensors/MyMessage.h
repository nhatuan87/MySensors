/*
 The MySensors library adds a new layer on top of the RF24 library.
 It handles radio network routing, relaying and ids.

 Created by Henrik Ekblad <henrik.ekblad@gmail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#ifndef MyMessage_h
#define MyMessage_h

#include <SPI.h>

#define PROTOCOL_VERSION 2
#define MAX_MESSAGE_LENGTH 32
#define HEADER_SIZE 8
#define MAX_PAYLOAD (MAX_MESSAGE_LENGTH - HEADER_SIZE)

// Message types
typedef enum {
	C_PRESENTATION = 0,
	C_SET = 1,
	C_REQ = 2,
	C_INTERNAL = 3,
	C_STREAM = 4 // For Firmware and other larger chunks of data that need to be divided into pieces.
} command;

// Type of sensor data (for set/req/ack messages)
typedef enum {
	V_TEMP,V_HUM, V_LIGHT, V_DIMMER, V_PRESSURE, V_FORECAST, V_RAIN,
	V_RAINRATE, V_WIND, V_GUST, V_DIRECTION, V_UV, V_WEIGHT, V_DISTANCE,
	V_IMPEDANCE, V_ARMED, V_TRIPPED, V_WATT, V_KWH, V_SCENE_ON, V_SCENE_OFF,
	V_HEATER, V_HEATER_SW, V_LIGHT_LEVEL, V_VAR1, V_VAR2, V_VAR3, V_VAR4, V_VAR5,
	V_UP, V_DOWN, V_STOP, V_IR_SEND, V_IR_RECEIVE, V_FLOW, V_VOLUME, V_LOCK_STATUS
} data;

// Type of internal messages (for internal messages)
typedef enum {
	I_BATTERY_LEVEL, I_TIME, I_VERSION, I_ID_REQUEST, I_ID_RESPONSE,
	I_INCLUSION_MODE, I_CONFIG, I_PING, I_PING_ACK,
	I_LOG_MESSAGE, I_CHILDREN, I_SKETCH_NAME, I_SKETCH_VERSION
} internal;

// Type of sensor  (for presentation message)
typedef enum {
	S_DOOR, S_MOTION, S_SMOKE, S_LIGHT, S_DIMMER, S_COVER, S_TEMP, S_HUM, S_BARO, S_WIND,
	S_RAIN, S_UV, S_WEIGHT, S_POWER, S_HEATER, S_DISTANCE, S_LIGHT_LEVEL, S_ARDUINO_NODE,
	S_ARDUINO_REPEATER_NODE, S_LOCK, S_IR, S_WATER, S_AIR_QUALITY
} sensor;

// Type of data stream  (for streamed message)
typedef enum {
	ST_FIRMWARE, ST_SOUND, ST_IMAGE
} stream;

typedef enum {
	P_STRING, P_BYTE, P_INT16, P_UINT16, P_LONG32, P_ULONG32, P_CUSTOM
} payload;



#define BIT(n)                  ( 1<<(n) )
// Create a bitmask of length len.
#define BIT_MASK(len)           ( BIT(len)-1 )
// Create a bitfield mask of length starting at bit 'start'.
#define BF_MASK(start, len)     ( BIT_MASK(len)<<(start) )
// Prepare a bitmask for insertion or combining.
#define BF_PREP(x, start, len)  ( ((x)&BIT_MASK(len)) << (start) )
// Extract a bitfield of length len starting at bit 'start' from y.
#define BF_GET(y, start, len)   ( ((y)>>(start)) & BIT_MASK(len) )
// Insert a new bitfield value x into y.
#define BF_SET(y, x, start, len)    ( y= ((y) &~ BF_MASK(start, len)) | BF_PREP(x, start, len) )

// Getters/setters for special fields in header
#define mSetVersion(_msg,_version) BF_SET(_msg.version_length, _version, 0, 3)
#define mSetLength(_msg,_length) BF_SET(_msg.version_length, _length, 3, 5)

#define mSetCommand(_msg,_command) BF_SET(_msg.command_ack_payload, _command, 0, 3)
#define mSetAck(_msg,_command) BF_SET(_msg.command_ack_payload, _command, 3, 1)
#define mSetPayloadType(_msg, _pt) BF_SET(_msg.command_ack_payload, _pt, 4, 4)

#define mGetVersion(_msg) BF_GET(_msg.version_length, 0, 3)
#define mGetLength(_msg) BF_GET(_msg.version_length, 3, 5)

#define mGetCommand(_msg) BF_GET(_msg.command_ack_payload, 0, 3)
#define mGetAck(_msg) BF_GET(_msg.command_ack_payload, 3, 1)
#define mGetPayloadType(_msg) BF_GET(_msg.command_ack_payload, 4, 4)


// internal access for special fields
#define miGetPayloadType() BF_GET(command_ack_payload, 4, 4)
#define miGetLength() BF_GET(version_length, 3, 5)

#define miSetLength(_length) BF_SET(version_length, _length, 3, 5)
#define miSetPayloadType(_pt) BF_SET(command_ack_payload, _pt, 4, 4)


class MyMessage
{
public:
	// Constructors
	MyMessage();

	MyMessage(uint8_t sensor, uint8_t type);

	/**
	 * If payload is something else than P_STRING you can have the payload value converted
	 * into string representation by supplying a buffer with the minimum size of
	 * 2*MAX_PAYLOAD+1. This is to be able to fit hex-conversion of a full binary payload.
	 */
	char* getString(char *buffer) const;
	const char* getString() const;
	void* getCustom() const;
	uint8_t getByte() const;
	bool getBool() const;
	double getDouble() const;
	long getLong() const;
	unsigned long getULong() const;
	int getInt() const;
	unsigned int getUInt() const;

	// Setters for building message "on the fly"
	MyMessage& setType(uint8_t type);
	MyMessage& setSensor(uint8_t sensor);
	MyMessage& setDestination(uint8_t destination);

	// Setters for payload
	MyMessage& set(void* payload, uint8_t length);
	MyMessage& set(const char* value);
	MyMessage& set(uint8_t value);
	MyMessage& set(double value, uint8_t decimals);
	MyMessage& set(unsigned long value);
	MyMessage& set(long value);
	MyMessage& set(unsigned int value);
	MyMessage& set(int value);


	uint8_t version_length;      // 3 bit - Protocol version
			                     // 5 bit - Length of payload
	uint8_t command_ack_payload; // 3 bit - Command type
	                             // 1 bit - Indicator that receiver should send an ack back.
	                             // 4 bit - Payload data type
	uint8_t sender;          	 // 8 bit - Id of sender node
	uint8_t last;            	 // 8 bit - Id of last node this message passed
	uint8_t destination;     	 // 8 bit - Id of destination node
	uint8_t type;            	 // 8 bit - Type varies depending on command
	uint8_t sensor;          	 // 8 bit - Id of sensor that this message concerns.

	// Each message can transfer a payload. We add one extra byte for string
	// terminator \0 to be "printable" this is not transferred OTA
	// This union is used to simplify the construction of the binary transferred int/long values.
	union {
		uint8_t bValue;
		unsigned long ulValue;
		long lValue;
		unsigned int uiValue;
		int iValue;
		char data[MAX_PAYLOAD + 1];
	} __attribute__((packed));
} __attribute__((packed));

#endif