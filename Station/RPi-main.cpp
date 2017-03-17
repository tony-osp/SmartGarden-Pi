/*

SmartGarden system.

This project was inspired by Ray's OpenSprinkler project, and by Sprinklers_pi program from Richard Zimmerman

Most of the code is written by Tony-osp (http://tony-osp.dreamwidth.org/),

*/


#include "core.h"
#include "settings.h"
#include <unistd.h>
#include <signal.h>
#include "Defines.h"
#include "port.h"
#include <Time.h>
#include "LocalUI.h"
#include "sdlog.h"
#include "LocalBoard.h"
#include "RProtocolMS.h"
#ifdef HW_ENABLE_XBEE
#include "XBeeRF.h"
#endif //HW_ENABLE_XBEE
#ifdef HW_ENABLE_SGGRF
#include "SGGRF.h"
#endif //HW_ENABLE_SGGRF


void setup();
void	RegisterRemoteEvents(void);

bool bTermSignal = false;

void signal_callback_handler(int signum)
{
   printf("Caught signal %d\n",signum);
   bTermSignal = true;

}

void signal_pipe_callback_handler(int signum)
{
   printf("Caught and ignored signal %d\n",signum);
}

int main(int argc, char **argv)
{
	// Register signal handlers
	signal(SIGTERM, signal_callback_handler);
	signal(SIGINT, signal_callback_handler);
	signal(SIGPIPE, signal_pipe_callback_handler);

	char * logfile = 0;
	int c = -1;
	while ((c = getopt(argc, argv, "?L:Vv")) != -1)
		switch (c)
		{
		case 'L':
			logfile = optarg;
			break;
		case 'V':
		case 'v':
			fprintf(stderr, "Version %u\n", SG_FIRMWARE_VERSION);
			return 0;
			break;
        case '?':
          if (optopt == 'L')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
          else
            fprintf (stderr, "Usage: %s [ -L(LOGFILE) ]'.\n", argv[0]);
          return 1;
        default:
          return 1;
		}

	if (logfile)
	{
		if (freopen(logfile, "a", stdout) == 0)
		{
			trace("FILE OUTPUT to %s FAILED!\n", logfile);
			return 1;
		}
	}
	//trace("Starting v%u..\n", SG_FIRMWARE_VERSION);

	wiringPiSetup();	// initialize wiringPi library
	setup();			// let SmartGarden system setup things

	while (!bTermSignal)
	{
		mainLoop();
		localUI.loop();
		rprotocol.loop();

	#ifdef SG_WDT_ENABLED
		SgWdtReset();
	#endif // SG_WDT_ENABLED

		// if we've changed the settings, store them to disk
        EEPROM.Store();

		usleep(1000);  // sleep for 1 ms
	}

	SYSEVT_CRIT("System shutdown.");
    localUI.lcd_print_line_clear_pgm(PSTR("   SmartGarden"), 0);
    localUI.lcd_print_line_clear_pgm(PSTR("...Turned Off..."), 1);

	trace("Exit\n");
	return 0;
}

OSLocalUI localUI;
SdFat sd;

void setup() {

#ifdef SG_WDT_ENABLED
	wdt_disable();
#endif // SG_WDT_ENABLED

	TRACE_CRIT(F("Start!\n"));
    localUI.begin();
  
	if (IsFirstBoot())
	{
		ResetEEPROM();	// note: ResetEEPROM will also reset the controller
	}
	
#ifdef HW_ENABLE_XBEE
    localUI.lcd_print_line_clear_pgm(PSTR("XBee init..."), 1);
	XBeeRF.begin();
#endif //HW_ENABLE_XBEE

#ifdef HW_ENABLE_SGGRF
    localUI.lcd_print_line_clear_pgm(PSTR("SG GW init..."), 1);
	SGGRF.begin();
#endif //HW_ENABLE_SGGRF

// network init for RPi
	uint16_t port = GetWebPort();
	if ((port > 65000) || (port < 80))
		port = 80;
	TRACE_INFO(F("Listening on Port %u\n"), port);

	pEthernet = new EthernetServer(port);
	pEthernet->begin();

	RegisterRemoteEvents();
}

