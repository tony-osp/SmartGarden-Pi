// SGGRF.cpp
/*
        SmartGarden Gateway RF support for SmartGarden

		Note: SmartGarden Gateway is emulated as XBee with 16bit addressing. A sub-set of commands is supported.

Creative Commons Attribution-ShareAlike 3.0 license
Copyright 2014-2016 tony-osp (http://tony-osp.dreamwidth.org/)
 
*/

#include "SGGRF.h"
#include "XBee.h"
#include "settings.h"
#include "RProtocolMS.h"

//#define TRACE_LEVEL			6		// trace everything for this module
#include "port.h"

extern int16_t		LastReceivedRSSI;	//extern, defined in RProtocolMS


//// Global core XBee object
XBee	xbee;				
SerialClass Serial;

// Local forward declarations
bool SGGRFSendPacket(uint8_t nStation, void *msg, uint8_t mSize);


// Start XBee network if enabled. 
//
// Note: It is assumed that EEPROM is already populated and valid.
//
SGGRFClass::SGGRFClass()
{
	fXBeeReady = false;
}

void SGGRFClass::begin()
{
	TRACE_INFO(F("Starting SG GW\n"));

	if( !IsXBeeEnabled() ){

		SYSEVT_ERROR(F("SG GW is not enabled, exiting\n"));
		return;					// init the system only if XBee is enabled.
	}

	uint8_t		xbeePort = GetXBeePort();
	uint16_t	xbeeSpeed = GetXBeePortSpeed();

	TRACE_INFO(F("SG GW port: %d, speed: %u\n"), xbeePort, xbeeSpeed);

// Set SGGW port and speed
	Serial.begin(xbeeSpeed); xbee.setSerial(Serial);	// on Uno always uses Serial

// Now set PAN ID, address and other parameters

	TRACE_INFO(F("Setting SG GW Addr: %X, PANID:%u, Chan:%u\n"), GetXBeeAddr(), GetXBeePANID(), (int)GetXBeeChan() );

	if( !sendAtCommandParam(PSTR("ID"), GetXBeePANID()) )
	{
		SYSEVT_ERROR(F("SG GW init - failed to set PAN ID"));
		goto failed_ex1;
	}
	if( !sendAtCommandParam(PSTR("CH"), GetXBeeChan()) )
	{
		SYSEVT_ERROR(F("SG GW init - failed to set Xbee Chan"));
		goto failed_ex1;
	}
	if( !sendAtCommandParam(PSTR("MY"), GetXBeeAddr()) )
	{
		SYSEVT_ERROR(F("SG GW init - failed to set Xbee Addr"));
		goto failed_ex1;
	}
	if( !sendAtCommand(PSTR("AC")) )
	{
		SYSEVT_ERROR(F("SG GW init - failed to execute AC command"));
		goto failed_ex1;
	}

	SetXBeeFlags(GetXBeeFlags() | NETWORK_FLAGS_ON);	// Mark SG GW network as On
	fXBeeReady = true;		// and set local readiness flag

	return;

failed_ex1:
	return;
}





bool SGGRFClass::sendAtCommand(const char *cmd_pstr) 
{
	uint8_t	  cmd[2];

	cmd[0] = pgm_read_byte(cmd_pstr);
	cmd[1] = pgm_read_byte(cmd_pstr+1);

	AtCommandRequest atRequest = AtCommandRequest(cmd);
	AtCommandResponse atResponse = AtCommandResponse();

	xbee.send(atRequest);

	if( xbee.readPacket(3000) )		// wait up to 3 seconds for the status response
	{
		// got a response! 
		if( xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE )	// it should be an AT command response
		{  
			xbee.getResponse().getAtCommandResponse(atResponse);
			if( atResponse.isOk() ) 
			{
				return true;	// exit - success
			} 
			else 
			{
				SYSEVT_ERROR(F("sendAtCommand - Command return error code: %X"), atResponse.getStatus());
			}
		} 
		else {
			SYSEVT_ERROR(F("sendAtCommand - Expected AT response but got %X"), xbee.getResponse().getApiId());
		}   
	} 
	else 
	{
		// at command failed
		if (xbee.getResponse().isError()) 
		{
			SYSEVT_ERROR(F("sendAtCommand - Error reading packet.  Error code: %d"), xbee.getResponse().getErrorCode());
		} 
		else {
			SYSEVT_ERROR(F("sendAtCommand - No response from radio"));  
		}
	}

	return false;	// failure. successful exit was above
}

bool	SGGRFClass::sendAtCommandParam(const char *cmd_pstr, uint8_t param8)
{
	return sendAtCommandParam(cmd_pstr, &param8, 1);
}

// Note: XBee uses big endian convention (MSB first), while Arduino is little endian system.
// We need to swap bytes.

bool	SGGRFClass::sendAtCommandParam(const char *cmd_pstr, uint16_t param16)
{
	uint8_t	 buf2[2];
	register uint8_t	 * pbuf = (uint8_t *)(&param16);
	buf2[0] = pbuf[1];
	buf2[1] = pbuf[0];

	return sendAtCommandParam(cmd_pstr, buf2, 2);
}

// Note: XBee uses big endian convention (MSB first), while Arduino is little endian system.
// We need to reorder bytes.
bool	SGGRFClass::sendAtCommandParam(const char *cmd_pstr, uint32_t param32)
{
	uint8_t	 buf4[4];
	register uint8_t	 * pbuf = (uint8_t *)(&param32);
	buf4[0] = pbuf[3];
	buf4[1] = pbuf[2];
	buf4[2] = pbuf[1];
	buf4[3] = pbuf[0];

	return sendAtCommandParam(cmd_pstr, buf4, 4);
}


bool SGGRFClass::sendAtCommandParam(const char *cmd_pstr, uint8_t *param, uint8_t param_len) 
{
	uint8_t	  cmd[2];

	cmd[0] = pgm_read_byte(cmd_pstr);
	cmd[1] = pgm_read_byte(cmd_pstr+1);

	AtCommandRequest atRequest = AtCommandRequest(cmd, param, param_len);
	AtCommandResponse atResponse = AtCommandResponse();

	xbee.send(atRequest);

	if( xbee.readPacket(3000) )		// wait up to 3 seconds for the status response
	{
		// got a response! 
		if( xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE )	// it should be an AT command response
		{  
			xbee.getResponse().getAtCommandResponse(atResponse);
			if( atResponse.isOk() ) 
			{
				return true;		// exit - success
			} 
			else 
			{
				SYSEVT_ERROR(F("sendAtCommandParam - Command return error code: %X"), atResponse.getStatus());
			}
		} 
		else {
			SYSEVT_ERROR(F("sendAtCommandParam - Expected AT response but got %X"), xbee.getResponse().getApiId());
		}   
	} 
	else 
	{
		// at command failed
		if (xbee.getResponse().isError()) 
		{
			SYSEVT_ERROR(F("sendAtCommandParam - Error reading packet.  Error code: %d"), xbee.getResponse().getErrorCode());
		} 
		else {
			SYSEVT_ERROR(F("sendAtCommandParam - No response from radio"));  
		}
	}

	return false;	// failure. successful exit was above
}

// Main packet send routine. 
//
//

bool SGGRFSendPacket(uint8_t nStation, void *msg, uint8_t mSize)
{
	if( !SGGRF.fXBeeReady )	// check that XBee is initialized and ready
		return false;

	TRACE_INFO(F("SG GW - sending packet to station %d, len %u\n"), nStation, (unsigned int)mSize);

	static Tx16Request tx = Tx16Request();						// pre-allocated, static objects to avoid dynamic memory issues
	static TxStatusResponse txStatus = TxStatusResponse();

	tx.setAddress16(nStation);								// since our XBee objects are pre-allocated, set parameters on the existing objects
	tx.setPayload((uint8_t *)msg);
	tx.setPayloadLength(mSize);

	if( nStation == STATIONID_BROADCAST )tx.setOption(8);   // send broadcast
	else								 tx.setOption(0);	// send unicast

	SGGRF.frameIDCounter++;	// increment rolling counter
	tx.setFrameId(0);			// set FrameID to 0, which means that XBee will not give us TX confirmation response.

    xbee.send(tx);

// No response from XBee is requested

	return true;
}


// Main SG GW loop poller. loop() should be called frequently, to allow processing of incoming packets
//
// Loop() will process incoming packets, will interpret remote protocol and will call appropriate handlers.
//
// Data is coming from XBee via serial port, and is buffered there (I think buffer length is 64 bytes), actual data pickup and interpretation
// happens in the loop() funciton.
//
// Note: Both incoming RF packets and send responses are picked up here.


void SGGRFClass::loop(void)
{
	if( !fXBeeReady ) return;	// XBee is not enabled, exiting.

	xbee.readPacket();	// read packet with no wait

	if( xbee.getResponse().isAvailable() )
	{
		// got a response! 
		register uint8_t	responseID = xbee.getResponse().getApiId();

		if(  responseID == TX_STATUS_RESPONSE )			// if we received TX status response
		{  
				// ignore TX responses (for now)
				return;		// exit 
		} 
		else if( responseID == RX_16_RESPONSE )			// actual data packet coming from a remote station
		{											
				static	Rx16Response rx16 = Rx16Response();		// Note: we are using 16bit addressing
				
				xbee.getResponse().getRx16Response(rx16);

				uint8_t  *msg = rx16.getFrameData();
				uint8_t	 msg_len = rx16.getFrameDataLength();

				if( msg_len < 5 )
				{
					SYSEVT_ERROR(F("SG GW.loop - incoming packet from station %d is too small"), rx16.getRemoteAddress16());
					return;
				}

				LastReceivedRSSI = -int16_t(rx16.getRssi());		// RSSI is reported as 8bit value but assumed to be negative
				TRACE_VERBOSE(F("SG GW.loop - processing packet from station %d\n"), rx16.getRemoteAddress16());
				rprotocol.ProcessNewFrame(msg+4, msg_len-4, 0 );	// process incoming packet.
																		// Note: we don't copy the packet, and just use pointer to the packet already in XBee library buffer
				return;
		} 
		else if( responseID == ZB_RX_RESPONSE )			// XBee Pro 900 uses this packet format when receiving 
		{											
				uint8_t  *msg = xbee.getResponse().getFrameData();
				uint8_t	 msg_len = xbee.getResponse().getFrameDataLength();

				static ZBRxResponse rx64 = ZBRxResponse();		// Note: we are using 64bit addressing
				xbee.getResponse().getZBRxResponse(rx64);
				
//				uint16_t  r16Addr = rx64.getRemoteAddress64().getLsb();

//				SYSEVT_ERROR(F("XBee.loop - received packed from %d, len=%d"), r16Addr, (int)msg_len);
				if( msg_len < 12 )
				{
					SYSEVT_ERROR(F("SG GW.loop - incoming packet is too small"));
					return;
				}

				rprotocol.ProcessNewFrame(msg+11, msg_len-11, msg);		// process incoming packet.
																		// Note: we don't copy the packet, and just use pointer to the packet already in XBee library buffer
				return;
		} 

		else if (responseID == 0x0F0)	// SG GW syslog frame
		{
			uint8_t  *msg = xbee.getResponse().getFrameData();
			uint8_t	 msg_len = xbee.getResponse().getFrameDataLength();

			if (msg_len < 5)
			{
				SYSEVT_ERROR(F("SG GW.loop - incoming syslog message from SG GW is too small"));
				return;
			}

			// first byte of the payload will be severity code, the rest is null-terminated actual message

			if( msg[0] <= SYSEVENT_CRIT )
			{
				SYSEVT_CRIT(F("SG GW: %.*s"), msg_len-5, msg + 1); // msg_len includes 4 bytes of framing (msb, lsb, API ID and frameID) as well as first byte of payload is the severity code
			}
			else if( msg[0] == SYSEVENT_ERROR )
			{
				SYSEVT_ERROR(F("SG GW: %.*s"), msg_len - 5, msg + 1);
			}
			else if (msg[0] <= SYSEVENT_INFO)
			{
				SYSEVT_INFO(F("SG GW: %.*s"), msg_len - 5, msg + 1);
			}
			else
			{
				SYSEVT_VERBOSE(F("SG GW: %.*s"), msg_len - 5, msg + 1);
			}

			return;
		}

		else {
			SYSEVT_ERROR(F("SG GW.loop - Unrecognized frame from SG GW %d"), responseID);
			return;
		}   
	} 
	else if( xbee.getResponse().isError() ) 
	{
		SYSEVT_ERROR(F("SG GW.loop - SG GW reported error.  Error code: %d"), xbee.getResponse().getErrorCode());
		return;
	}

	// Note: SG GW error status and current packet will be discarded on the new readPacket(), on the next loop() pass.
}

SGGRFClass SGGRF;

