// MoteinoRF.h
/*
        Moteino RF support for SmartGarden


Creative Commons Attribution-ShareAlike 3.0 license
Copyright 2015 tony-osp (http://tony-osp.dreamwidth.org/)

*/

#ifndef _MOTEINORF_h
#define _MOTEINORF_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Defines.h"
#include <RFM69.h>

//#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
#define MOTEINORF_FREQUENCY       RF69_915MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
//#define ENCRYPTKEY      "sampleEncryptKey" //has to be same 16 characters/bytes on all nodes, not more not less!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!

#define NETWORK_MOTEINORF_RETRY_COUNT		3
#define NETWORK_MOTEINORF_USE_INTERRUPT		1		// use interrupts


// ***TEMPORARY***
// RFM69 encryption key is statically defined here (16 characters)
#define MOTEINORF_ENCRYPTKEY	"SmartGarden v1.x"

class MoteinoRFClass
{
 public:
	MoteinoRFClass();
	void begin(void);
	void loop(void);

	bool setPANID(uint8_t);		// set PAN ID
	bool setNodeID(uint8_t);	// set network node ID
	bool setChanID(uint8_t);	// set network node ID

	bool sendPacket(uint8_t dest, bool fDisableACK, void *msg, uint8_t mSize);

	bool	fMoteinoRFReady;			// Flag indicating that XBee is initialized and ready
	uint8_t	uNextSNumber[MAX_STATIONS];
	uint8_t	uLastReceivedSNumber[MAX_STATIONS];

	uint8_t	nodeID;		// our node ID
	uint8_t	panID;

private:

};

extern MoteinoRFClass MoteinoRF;

#endif

