/*
        SmartGarden Gateway - Serial communication with the Host


Creative Commons Attribution-ShareAlike 3.0 license
Copyright 2016 tony-osp (http://tony-osp.dreamwidth.org/)

*/
#include "SGGSerial.h"
#include "Defines.h"
#include "MoteinoRF.h"
#include "port.h"

void SGGSerialClass::begin()
{
	Serial1.begin(HOST_SERCOMM_SPEED);	// we use Serial1 for communication with the Host
}

// Worker routines
// For code efficiency these routines are defined as inline

// input packet processed routine. It will be called from newChar() when complete packet is received 
inline void processNewPacket(uint8_t *buf, uint16_t buf_len)
{
	//TRACE_VERBOSE(F("SggSerial - received input packet with length=%u\n"), buf_len);

	if( buf_len < 1 )
	{
		TRACE_ERROR(F("SggSerial - received bad packet with length=%u\n"), buf_len);
		return;
	}

	switch( buf[0] )	// first byte of the packet is the API ID
	{
		case 0x8:		// AT Command
					SGGSerial.processATCommand(buf, buf_len);
					break;

		case 0x9:		// Queue AT Command
					SGGSerial.processATCommand(buf, buf_len);
					break;

		case 0x0:		// TX 64bit Command
					SGGSerial.processTX64Command(buf, buf_len);
					break;

		case 0x1:		// TX 16bit Command
					SGGSerial.processTX16Command(buf, buf_len);
					break;

		default:
					TRACE_ERROR(F("SggSerial - received unrecognized command %x\n"), uint16_t(buf[0]));
					break;
	}
}



// Input parser state and working variables
// Definitions
#define SGG_SERIAL_STATE_IDLE				0
#define SGG_SERIAL_STATE_START_RECEIVED		1
#define SGG_SERIAL_STATE_MSB_RECEIVED		2
#define SGG_SERIAL_STATE_LSB_RECEIVED		3

static	uint8_t		sggSerialState = SGG_SERIAL_STATE_IDLE;
static  uint16_t	sggSerialFrameLength = 0;
static  uint16_t	sggSerialFrameByteCounter = 0;
static  bool		sggSerialEscapeState = false;
static  uint8_t		sggSerialChecksum = 0;

// input buffer
static	uint8_t		sggSerialInputFrameBuffer[HOST_SERCOMM_MAX_FRAME];

// Main input processing routine. It is called once with every input character
//
inline void newChar(uint8_t c)
{
	if( c == 0x7e )
	{
		if( sggSerialState != SGG_SERIAL_STATE_IDLE )
		{
			TRACE_ERROR(F("SggSerial - framing error, received 0x7E while in non-idle state %u\n"), uint16_t(sggSerialState));
		}

		// new frame, initialize state
		sggSerialState = SGG_SERIAL_STATE_START_RECEIVED;
		sggSerialFrameLength = 0;
		sggSerialEscapeState = false;
		sggSerialChecksum = 0;
		sggSerialFrameByteCounter = 0;

		return;		// new frame initialized, exit
	}

	// received byte and it is not a new frame marker. Process escaping 

	if( !sggSerialEscapeState && (c == 0x7D) ) // entering escaped state
	{
		sggSerialEscapeState = true;
		return;
	}
	
	if( sggSerialEscapeState ) // previous byte was escape code
	{
		c = c ^ 0x20;
		sggSerialEscapeState = false;
	}

	// dispatch further processing based on parser state
	switch( sggSerialState )
	{
		case SGG_SERIAL_STATE_START_RECEIVED:
											sggSerialFrameLength = (c << 8) && 0x0FF00;	// we received MSB
											sggSerialState = SGG_SERIAL_STATE_MSB_RECEIVED;

											//TRACE_VERBOSE(F("SggSerial - got MSB, sggSerialFrameLength=%u\n"), uint16_t(sggSerialFrameLength));
											break;

		case SGG_SERIAL_STATE_MSB_RECEIVED:
											sggSerialFrameLength += c;	// we received LSB
											sggSerialState = SGG_SERIAL_STATE_LSB_RECEIVED;

											//TRACE_VERBOSE(F("SggSerial - got LSB, sggSerialFrameLength=%u\n"), uint16_t(sggSerialFrameLength));
											break;

		case SGG_SERIAL_STATE_LSB_RECEIVED:
											if( sggSerialFrameByteCounter == sggSerialFrameLength ) // we got all payload characters, expecting checksum
											{
												sggSerialChecksum += c;		// update checksum
												if( sggSerialChecksum == 0xFF )
												{
													// checksum correct, process packet
													processNewPacket(sggSerialInputFrameBuffer, sggSerialFrameByteCounter);
												}
												else
												{
													TRACE_ERROR(F("SggSerial - checksum error, received checksum byte %u and now checksum is %u. Byte Counter=%u\n"), uint16_t(c), uint16_t(sggSerialChecksum), uint16_t(sggSerialFrameByteCounter));
												}

												// existing frame processed, reset state back to idle
												sggSerialState = SGG_SERIAL_STATE_IDLE;
												break;
											}

											// this is not the end of the frame yet, update counter and checksum
											sggSerialChecksum += c;		

											if( sggSerialFrameByteCounter < (HOST_SERCOMM_MAX_FRAME-1) ) // basic protection from frame buffer overflow
											{
												sggSerialInputFrameBuffer[sggSerialFrameByteCounter] = c;	// save received byte
												sggSerialFrameByteCounter++;
											}
											else
											{
												TRACE_ERROR(F("SggSerial - too big frame error, ignoring input byte %u\n"), uint16_t(c));
											}

											break;

		case SGG_SERIAL_STATE_IDLE:									
											TRACE_ERROR(F("SggSerial - error, received byte %c while idle\n"), c);
											break;

		default:
					TRACE_ERROR(F("SggSerial - state error when processing new byte %u\n"), uint16_t(c));
					break;
	}

}


// input packet processing routine for AT commands 
void SGGSerialClass::processATCommand(uint8_t *buf, uint16_t buf_len)
{
	if( buf_len < 4 )
	{
		TRACE_ERROR(F("processATCommand - input buffer length of %u is too small\n"), buf_len);
		return;
	}

	// Byte 0 would be 0x8 or 0x9 (process AT command or queue AT command), byte 1 will be frame number, and bytes 2 and 3 will be actual AT command

	TRACE_VERBOSE(F("processATCommand - received command %c%c\n"), buf[2], buf[3]);

	uint8_t		frameID = buf[1];

	if( (buf[2] == 'I') && (buf[3] == 'D') )
	{
		//TRACE_VERBOSE(F("SggSerial - setting PANID=%u\n"), uint16_t(buf[5]));

		// ID command, which sets PAN ID
		bool fRet = MoteinoRF.setPANID(buf[5]);		// note: ID command has 16bit parameter, MSB first. Since MoteinoRF uses 8bit network and node addressing, we use LSB part only		
		sendATResponse(frameID, fRet, null, 0);	// send OK/fail response with no parameters
	}
	else if( ((buf[2] == 'C') && (buf[3] == 'H')) || ((buf[2] == 'H') && (buf[3] == 'P')) ) // the emulator accepts both CH (used by non-Pro) and HP (used by Pro) commands for channel number
	{
		//TRACE_VERBOSE(F("SggSerial - setting NodeID=%u\n"), uint16_t(buf[4]));

		// Channel command
		bool fRet = MoteinoRF.setChanID(buf[4]);
		sendATResponse(frameID, fRet, null, 0);	// send OK/fail response with no parameters
	}
	else if( (buf[2] == 'M') && (buf[3] == 'Y') )
	{
		// MY address command
		bool fRet = MoteinoRF.setNodeID(buf[5]);
		sendATResponse(frameID, fRet, null, 0);	// send OK/fail response with no parameters
	}
	else if( (buf[2] == 'A') && (buf[3] == 'C') )
	{
		// "Apply Changes" command. Restart MoteinoRF to apply changes
		MoteinoRF.begin();
		sendATResponse(frameID, true, null, 0);	// send OK response with no parameters
	}
	else
	{
		TRACE_ERROR(F("processATCommand - received unknown command %c%c\n"), buf[2], buf[3]);
	}
}

// input packet processing routine for Transmit with 16bit addr commands 
void SGGSerialClass::processTX16Command(uint8_t *buf, uint16_t buf_len)
{
	if( buf_len < 6 )	//TX16 command header/address + one byte of payload will be 6 bytes
	{
		TRACE_ERROR(F("processTX16Command - input buffer length of %u is too small\n"), buf_len);
		return;
	}

	uint8_t		frameID = buf[1];
	uint8_t		dest = buf[3];	// since RFM69 uses 8bit network addresses, we use LSB part of the 16bit address only
	uint8_t		opt = buf[4];	// options byte (0x01 - Disable ACK, 0x04 - send broadcast PAN ID);
	uint16_t    data_len = buf_len - 5;

	if( data_len > RF69_MAX_DATA_LEN )
	{
		TRACE_ERROR(F("processTX16Command - input data block length of %u is too big\n"), data_len);
		return;
	}

	bool retFlag = MoteinoRF.sendPacket( dest, (opt & 0x01)? true:false, &buf[5], uint8_t(data_len) );

	if( frameID != 0 )	// confirmation was requested
	{
		if( retFlag ) sendTXResponse( frameID, false, 1 ); // we use error code 1, "no ACK received" as a generic error response
		else		  sendTXResponse( frameID, true, 0 );  // success	
	}
}

// input packet processing routine for Transmit with 64bit addr commands 
void SGGSerialClass::processTX64Command(uint8_t *buf, uint16_t buf_len)
{
	if( buf_len < 8 )	//TX64 command header/address + one byte of payload will be 8 bytes
	{
		TRACE_ERROR(F("processTX64Command - input buffer length of %u is too small\n"), buf_len);
		return;
	}

	uint8_t		frameID = buf[1];
	uint8_t		dest = buf[5];	// since RFM69 uses 8bit network addresses, we use LSB part of the 64bit address only
	uint8_t		opt = buf[6];	// options byte (0x01 - Disable ACK, 0x04 - send broadcast PAN ID);
	uint16_t    data_len = buf_len - 7;

	if( data_len > RF69_MAX_DATA_LEN )
	{
		TRACE_ERROR(F("processTX64Command - input data block length of %u is too big\n"), data_len);
		return;
	}

	bool retFlag = MoteinoRF.sendPacket( dest, (opt & 0x01)? true:false, &buf[7], uint8_t(data_len) );

	if( frameID != 0 )	// confirmation was requested
	{
		if( retFlag ) sendTXResponse( frameID, false, 1 ); // we use error code 1, "no ACK received" as a generic error response
		else		  sendTXResponse( frameID, true, 0 );  // success	
	}
}

//
// Output (Gateway->Host) routines
//

// Send AT response with parameters
void SGGSerialClass::sendATResponse(uint8_t frameID, bool fOK, uint8_t cmd1, uint8_t cmd2)
{
	uint8_t	buf[3];
	buf[0] = cmd1; buf[1] = cmd2;

	if( fOK ) buf[2] = 0; // status OK
	else      buf[2] = 1; // status - error

	sendFrameToHost(0x88, frameID, buf, 3);
}

// Send TX response with parameters
void SGGSerialClass::sendTXResponse(uint8_t frameID, bool fOK, uint8_t status)
{
	uint8_t	buf[1];

	if( fOK ) buf[0] = 0; // status OK
	else      buf[0] = status; // status - error

	sendFrameToHost(0x89, frameID, buf, 1);
}


// inline helper
inline void sggSerialSend1byte(uint8_t c)
{
	if( (c==0x7E) || (c==0x7D) || (c==0x11) || (c==0x13) )  // this byte must be escaped
	{
		Serial1.write(uint8_t(0x7D));
		Serial1.write(uint8_t(c^0x20));
	}
	else
	{
		Serial1.write(uint8_t(c));
	}
}

// Worker "send response" routine
void SGGSerialClass::sendFrameToHost(uint8_t apiID, uint8_t frameID, uint8_t *buf, uint16_t buf_len)
{
	//TRACE_VERBOSE(F("SggSerial - sending frame to host. apiID=%x, frameID=%x, buflen=%u\n"), uint16_t(apiID), uint16_t(frameID), buf_len);
	
	uint8_t  checksum = 0;
		
	uint8_t len_msb = ((buf_len+2) >> 8) && 0xff;	// +2 to account for header
	uint8_t len_lsb = uint16_t(buf_len+2);

	Serial1.write(uint8_t(0x7E));	// start of the frame marker
	sggSerialSend1byte(len_msb);	// the rest of the frame must be escaped
	sggSerialSend1byte(len_lsb);

	sggSerialSend1byte(apiID);		checksum += apiID;
	sggSerialSend1byte(frameID);	checksum += frameID;

	uint8_t *ptr = buf;
	for( uint16_t i=0; i<buf_len; i++ )
	{
		sggSerialSend1byte(*ptr); checksum += *ptr++;
	}
	sggSerialSend1byte(uint8_t(0xff)-checksum);
}

void SGGSerialClass::sendRXFrameToHost(uint16_t sourceID, uint8_t rssi, uint8_t options, uint8_t *buf, uint16_t buf_len)
{
	uint8_t  checksum = 0;
	
	uint8_t len_msb = ((buf_len+5) >> 8) && 0xff;	// +5 to account for header
	uint8_t len_lsb = uint8_t(buf_len+5);

	uint8_t src_msb = (sourceID >> 8) && 0xff;
	uint8_t src_lsb = sourceID && 0xff;

	Serial1.write(uint8_t(0x7E));	// start of the frame marker
	sggSerialSend1byte(len_msb);	// the rest of the frame must be escaped
	sggSerialSend1byte(len_lsb);

	sggSerialSend1byte(0x81);		checksum += 0x81;
	sggSerialSend1byte(src_msb);	checksum += src_msb;
	sggSerialSend1byte(src_lsb);	checksum += src_lsb;

	sggSerialSend1byte(rssi);	checksum += rssi;
	sggSerialSend1byte(options);	checksum += options;

	uint8_t *ptr = buf;
	for( uint16_t i=0; i<buf_len; i++ )
	{
		sggSerialSend1byte(*ptr); checksum += *ptr++;
	}
	sggSerialSend1byte(uint8_t(0xff)-checksum);
}


// This function will be called regularly to allow serial comms module to get and process input
void SGGSerialClass::loop()
{
	register int	ic;

	while( Serial1.available() > 0 )	// process all available bytes in the input buffer 
	{
		ic = Serial1.read(); 

		//TRACE_VERBOSE(F("SggSerial - got byte %x\n"), ic);

		if( ic != -1 ) newChar(uint8_t(ic));
		else           TRACE_ERROR(F("Serial comms error - got -1 when reading from the Host\n"));
	}
}

SGGSerialClass SGGSerial;

