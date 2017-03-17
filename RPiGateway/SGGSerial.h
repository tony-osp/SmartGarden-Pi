// SGGSerial.h
/*
        SmartGarden Gateway - Serial communication with the Host


Creative Commons Attribution-ShareAlike 3.0 license
Copyright 2016 tony-osp (http://tony-osp.dreamwidth.org/)

*/
#ifndef _SGGSERIAL_h
#define _SGGSERIAL_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class SGGSerialClass
{
 public:
	void begin(void);

	//void newChar(char);
	void loop(void);

	//void processNewPacket(uint8_t *buf, uint16_t buf_len);
	void processATCommand(uint8_t *but, uint16_t buf_len);
	void processTX16Command(uint8_t *but, uint16_t buf_len);
	void processTX64Command(uint8_t *but, uint16_t buf_len);

	void sendATResponse(uint8_t frameID, bool fOK, uint8_t cmd1, uint8_t cmd2);
	void sendTXResponse(uint8_t frameID, bool fOK, uint8_t status);

	void sendRXFrameToHost(uint16_t sourceID, uint8_t rssi, uint8_t options, uint8_t *buf, uint16_t buf_len);

	void sendFrameToHost(uint8_t apiID, uint8_t frameID, uint8_t *buf, uint16_t buf_len);

};

extern SGGSerialClass SGGSerial;

#endif

