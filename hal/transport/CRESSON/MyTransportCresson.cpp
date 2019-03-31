/*
 * Created by Tuan Tran <tran.tuan@cmengineering.com.vn>
 * Copyright (C) 2014-2018 CM Engineering Vietnam Co. Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 ********************************************************************************/

// Cresson Transport
#if defined(MY_CRESSON_HWSERIAL)
	HardwareSerial& _dev = MY_CRESSON_HWSERIAL;
	#if defined(MY_CRESSON_POWER_PIN)
	crstream<>		cresson(_dev, MY_CRESSON_POWER_PIN);
	#else
	crstream<>		cresson(_dev);
	#endif
#else
	SoftwareSerial 	_dev(MY_CRESSON_TX_PIN, MY_CRESSON_RX_PIN);
	#if defined(MY_CRESSON_POWER_PIN)
		crstream<SoftwareSerial>		cresson(_dev, MY_CRESSON_POWER_PIN);
	#else
		crstream<SoftwareSerial>		cresson(_dev);
	#endif
#endif

uint8_t _packet_len;
int16_t _rcv_rssi;
char _data[MY_CRESSON_MAX_MESSAGE_LENGTH];

void cresson_onReceived() {
	_rcv_rssi   = cresson.rssi();
}

void cresson_onReceiving() {
	if (cresson.length() <= MY_CRESSON_MAX_MESSAGE_LENGTH) {
		while (cresson.available()) cresson >> _data[_packet_len++];
	}
}

bool transportSend(const uint8_t to, const void* data, const uint8_t len, const bool noACK)
{
	(void)noACK;	// not implemented
	const char *datap = static_cast<char const *>(data);
	cresson.destID = (to == BROADCAST_ADDRESS) ? (uint16_t) BROADCAST_ID : (uint16_t) to;
	for (uint8_t i=0; i<len; i++) {
		cresson << datap[i];
	}
	cresson.listen();
	return true;
}

bool transportInit(void)
{
  	cresson.begin();
	return cresson.isAlive();
}

void transportSetAddress(const uint8_t address)
{
	cresson.selfID = (uint16_t) address;
	cresson.writecmd(P_sysreg, 2, 0x01, address);
	cresson.execute();
}

uint8_t transportGetAddress(void)
{
	return cresson.selfID;
}

bool transportAvailable(void)
{
	return cresson.listen() and _packet_len > 0;
}

bool transportSanityCheck(void)
{
	// not implemented yet
	return true;
}

uint8_t transportReceive(void* data)
{
	uint8_t len = _packet_len;
	if (_packet_len > 0) {
		memcpy(data,_data,_packet_len);
		_packet_len = 0;
	}
	return len;
}

void transportPowerDown(void)
{
	cresson.powerOff();
}

void transportPowerUp(void)
{
	cresson.powerOn();
}
void transportSleep(void)
{
	cresson.sleep();
}
void transportStandBy(void)
{
	cresson.wakeup();
}

int16_t transportGetSendingRSSI(void)
{
	// not implemented
	return INVALID_RSSI;
}

int16_t transportGetReceivingRSSI(void)
{
	return _rcv_rssi;
}

int16_t transportGetSendingSNR(void)
{
	// not implemented
	return INVALID_SNR;
}

int16_t transportGetReceivingSNR(void)
{
	// not implemented
	return INVALID_SNR;
}

int16_t transportGetTxPowerPercent(void)
{
	// not implemented
	return static_cast<int16_t>(100);
}

int16_t transportGetTxPowerLevel(void)
{
	// not implemented
	return static_cast<int16_t>(100);
}

bool transportSetTxPowerPercent(const uint8_t powerPercent)
{
	// not possible
	(void)powerPercent;
	return false;
}
