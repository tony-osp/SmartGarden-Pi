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

void setup()
{
#ifdef SG_WDT_ENABLED
	wdt_disable();
#endif // SG_WDT_ENABLED

	trace_setup(Serial, 115200);		// we use Serial0 for debug
	TRACE_CRIT(F("Start!\n"));

    TRACE_INFO(F("MoteinoRF init\n"));
	MoteinoRF.begin();
	
    TRACE_INFO(F("SGGSerial init\n"));
	SGGSerial.begin();

}

// Main loop
void loop()
{
	SGGSerial.loop();
	MoteinoRF.loop();
}
