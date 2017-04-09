/*

SmartGarden system.

This project was inspired by Ray's OpenSprinkler project, and by Sprinklers_pi program from Richard Zimmerman

This particular module (SmartGarden Gateway) is connecting Raspberry Pi or other controller to RF network, 
allowing it to control remote SmartGarden stations and to collect data from them.

The Gateway also provides Raspberry Pi access to Analog signals and GPIO pins, allowing to get data from analog sensors (e.g. Thermistor sensor).

Creative Commons Attribution-ShareAlike 3.0 license
Copyright 2014-2016 tony-osp (http://tony-osp.dreamwidth.org/)

*/
#include "SGGSerial.h"
#include "Defines.h"
#include "port.h"

#include <ostream.h>
#include <istream.h>
#include <iostream.h>
#include <ios.h>
#include <bufstream.h>
#include <ArduinoStream.h>

#include <SPI.h>
#include <EEPROM.h>
#include <Time.h>
#include <Wire.h>

#ifdef SG_WDT_ENABLED
#include <avr/wdt.h>
#include "SgWdt.h"
#endif // SG_WDT_ENABLED

#include <RFM69.h>
#include "MoteinoRF.h"

#include <TimerOne.h>

int16_t		LastReceivedRSSI;

static const char * const sHeader = SG_FIRMWARE_SHEADER;

bool CheckEEPROMHeader()
{
		if( (EEPROM.read(ADDR_SHEADER) == sHeader[0]) && (EEPROM.read(ADDR_SHEADER+1) == sHeader[1]) && (EEPROM.read(ADDR_SHEADER+2) == sHeader[2]) && (EEPROM.read(ADDR_SHEADER+3) == sHeader[3]) )
            return true;

        return false;
}

void SaveEEPROMHeader()
{
        for (int i = 0; i <= 3; i++)			// write current signature
                EEPROM.write(ADDR_SHEADER+i, sHeader[i]);

		EEPROM.write(ADDR_VERSION_MAJOR, SG_FIRMWARE_VERSION_MAJOR);
		EEPROM.write(ADDR_VERSION_MINOR, SG_FIRMWARE_VERSION_MINOR);
}



void setup()
{
#ifdef SG_WDT_ENABLED
	wdt_disable();
#endif // SG_WDT_ENABLED

	trace_setup(Serial, 115200);		// we use Serial0 for debug

	if( CheckEEPROMHeader() )
	{
		TRACE_INFO(F("RPiGateway Starting, FW %d.%d\n"), EEPROM.read(ADDR_VERSION_MAJOR), EEPROM.read(ADDR_VERSION_MINOR));
	}
	else
	{
		SaveEEPROMHeader();

		EEPROM.write(ADDR_NODE_ID, DEFAULT_STATION_ID);
		EEPROM.write(ADDR_PAN_ID, NETWORK_MOTEINORF_DEFAULT_PANID);

		TRACE_INFO(F("RPiGateway Starting and initializing EEPROM, FW %d.%d\n"), EEPROM.read(ADDR_VERSION_MAJOR), EEPROM.read(ADDR_VERSION_MINOR));
	}

	MoteinoRF.begin();
	
	SGGSerial.begin();

}

// Main loop
void loop()
{
	SGGSerial.loop();
	MoteinoRF.loop();
}
