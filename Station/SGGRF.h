// SGGRF.h
/*
        SmartGarden Gateway RF support for SmartGarden




Creative Commons Attribution-ShareAlike 3.0 license
Copyright 2014 tony-osp (http://tony-osp.dreamwidth.org/)

*/

#ifndef _SGGRF_h
#define _SGGRF_h

#include "XBee.h"
#include "Defines.h"

class SGGRFClass
{
 public:
	SGGRFClass();
	void begin(void);
	void loop(void);

	bool	fXBeeReady;			// Flag indicating that XBee is initialized and ready
	uint8_t	frameIDCounter;		// Rolling counter used to generate FrameID

private:

	bool	sendAtCommand(const char *cmd_pstr);
	bool	sendAtCommandParam(const char *cmd_pstr, uint8_t *param, uint8_t param_len);
	bool	sendAtCommandParam(const char *cmd_pstr, uint8_t param8);
	bool	sendAtCommandParam(const char *cmd_pstr, uint16_t param16);
	bool	sendAtCommandParam(const char *cmd_pstr, uint32_t param32);

};

extern SGGRFClass SGGRF;
bool SGGRFSendPacket(uint8_t nStation, void *msg, uint8_t mSize);

#endif

