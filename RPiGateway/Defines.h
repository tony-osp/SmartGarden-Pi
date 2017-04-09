/*

Defaults definition for the Gateway module of the SmartGarden system.


Creative Commons Attribution-ShareAlike 3.0 license
Copyright 2016 tony-osp (http://tony-osp.dreamwidth.org/)

*/

#ifndef _DEFINES_H
#define _DEFINES_H 

#define ENABLE_TRACE	1		// enable trace output (via Serial port)
//#define TRACE_LEVEL		1		// critical messages only
#define TRACE_LEVEL		2		// critical and error messages only
//#define TRACE_LEVEL			7		// all info
#define TRACE_FREERAM_LIMIT	2000	// when free RAM goes below this limit freeMem() calls will start producing critical notifications

// System events level threshold. System events are written to the system log, also they are copied to the trace output.
//#define SYSEVT_LEVEL	2	// critical events only
#define SYSEVT_LEVEL	3	// errors and critical events only

#define HOST_SERCOMM_SPEED		57600	// serial port speed for communication with the Host
#define HOST_SERCOMM_MAX_FRAME	128		// maximum 128 bytes

// Supported hardware version definitions

#define HW_V20_GATEWAY			1	// SmartGarden Gateway, hardware version 2.0 (Moteino Mega with RFM69HW RF module)

//To select specific hardware version uncomment the line below corresponding to required HW version.

#define SG_HARDWARE				HW_V20_GATEWAY


// This section defined macro-level HW config for different versions.
// Please note that part of the HW config (e.g. specific pin assignments etc) is defined in HardwiredConfig.h file.

#if SG_HARDWARE == HW_V20_GATEWAY

#define HW_ENABLE_MOTEINORF		1
#define DEFAULT_STATION_ID		9	// Usually SG Gateway will be used on Master. But in any case Host should explicitly set the station ID for RF network
#define NETWORK_MOTEINORF_DEFAULT_PANID	55


#endif //HW_V20_GATEWAY

#define MAX_REMOTE_STATIONS  9	// remote stations may have numbers from 1 to 9
#define MAX_STATIONS		 16 // Maximum number of stations 

#define SG_FIRMWARE_SHEADER "SGG2"
#define SG_FIRMWARE_VERSION_MAJOR	1
#define SG_FIRMWARE_VERSION_MINOR	0

#define ADDR_SHEADER		0
#define ADDR_VERSION_MAJOR	4
#define ADDR_VERSION_MINOR	5

#define ADDR_NODE_ID		6
#define ADDR_PAN_ID			7

#define STATIONID_BROADCAST			255		// reserved stationID for broadcasts

// Watchdog timer config
//#define SG_WDT_ENABLED			1	// enable WDT 

// Max number of watchdog timer ticks before reset
#define SG_WDT_MAX_TICK_ALLOWED 8	// 8 ticks
// Watchdog timer tick
#define SG_WDT_TICK	WDT_8S	// 8 seconds per tick


#endif // _DEFINES_H