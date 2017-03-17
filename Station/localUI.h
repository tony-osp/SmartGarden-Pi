/*
       Local UI (for local LCD and buttons) on SmartGarden

  This module is intended to be used with my multi-station environment monitoring and sprinklers control system (SmartGarden),
  as well as modified version of the OpenSprinkler code and with modified sprinkler control program sprinklers_pi.

  This module handles local UI (LCD and four input buttons connected to the microcontroller). Local UI shows status and allows limited
control of the controller and individual valves.

The code is waitless (does not use delay()), but should be called from loop() sufficiently frequently to handle input and update the UI.


Creative Commons Attribution-ShareAlike 3.0 license
Copyright 2014 tony-osp (http://tony-osp.dreamwidth.org/)

*/


#ifndef _OSLocalUI_h
#define _OSLocalUI_h

#ifdef ARDUINO
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#endif //ARDUINO

#include "Defines.h"
#include <Time.h>


#ifdef USE_I2C_LCD
#include "LCD_C0220BiZ.h"
#else
#ifdef ARDUINO
#include <LiquidCrystal.h>
#else
#include "LiquidCrystal.h"
#endif //ARDUINO
#endif

#ifdef USE_I2C_LCD
// I2C LCD library uses reverse parameters order for setcursor
#define LCD_SETCURSOR(lcd, x, y)	lcd.setCursor(y,x)

#else
// Standard LCD library setcursor conventions
#define LCD_SETCURSOR(lcd, x, y)	lcd->setCursor(x,y)

#endif

// global states definitions

// Modes
// Modes numbering starts from zero, and we could have up to 255 Modes, with the mode value of 255 reserved as "undefined"
#define OSUI_MODE_UNDEFINED             255             // undefined
#define OSUI_MODE_HOME                  0               // Home screen
#define OSUI_MODE_MANUAL                1               // Manual mode
#define OSUI_MODE_STATUS	            2               // View status
#define OSUI_MODE_SETUP                 3               // Setup mode

// Pages
// Pages numbering starts from zero, and each Mode has its own set of pages
#define OSUI_PAGE_UNDEFINED             255


// Overall UI state
#define OSUI_STATE_UNDEFINED			255
#define OSUI_STATE_SUSPENDED			0
#define OSUI_STATE_ENABLED              1



// ====== Button Defines ======
#define BUTTON_1            0x01
#define BUTTON_2            0x02
#define BUTTON_3            0x04
#define BUTTON_4            0x08

// button status values
#define BUTTON_NONE         0x00  // no button pressed
#define BUTTON_MASK         0x0F  // button status mask
#define BUTTON_FLAG_HOLD    0x80  // long hold flag
#define BUTTON_FLAG_DOWN    0x40  // down flag
#define BUTTON_FLAG_UP      0x20  // up flag



// Button meaning

#define BUTTON_MODE             BUTTON_1
#define BUTTON_UP               BUTTON_2
// note: button 3 is the optional button
#define BUTTON_DOWN             BUTTON_3
#define BUTTON_CONFIRM          BUTTON_4

class OSLocalUI {
public:

  // ====== Member Functions ======
  // -- Setup --
  uint8_t begin(void);                              // initialization. Intended to be called from setup()

    // -- Operation --
  uint8_t loop(void);                               // Main loop. Intended to be called regularly and frequently to handle input and UI. Normally this will be called from Arduino loop()
  uint8_t refresh(void);                            // Force UI refresh.
  uint8_t suspend(void);                            // Stop UI updates and input handling (useful when taking over the screen for custom output)
  uint8_t resume(void);                                             // Resume UI operation

  uint8_t set_mode(char mode);              // Switch UI to desired Mode
  void lcd_print_pgm(const prog_char * str);
  void lcd_print_line_clear_pgm(const prog_char * str, uint8_t line);    // Print a program memory string to a given line with clearing
  void lcd_print_2digit(int v);
  void lcd_print_3digit(int v);

// Data
#ifdef USE_I2C_LCD
  static ST7036 lcd;
#else
  //static LiquidCrystal lcd;             // Main LCD object. We have to expose this object to allow custom code access to LCD bypassing UI
  LiquidCrystal *lcd;             // Main LCD object. We have to expose this object to allow custom code access to LCD bypassing UI
#endif

  static uint8_t osUI_State;
  static uint8_t osUI_Mode;
  static uint8_t osUI_Page;

private:

// internal stuff

  static uint8_t display_board;                                                    // currently displayed board

  void lcd_print_time(uint8_t line);                // Print time to a given line
  void lcd_print_memory(uint8_t line);              // Print free memory
  void lcd_print_ip(const uint8_t *ip);             // print ip address and port
  void lcd_print_station(void);
  void lcd_print_station(char def_c, uint8_t sel_stn, char sel_c, uint8_t max_ch);


// Mode handlers.
// Normally called from loop(), parameter:   0 - regular loop call, no special handling
//                                                                  1 - force UI refresh     (i.e. screen may be corrupted)
//                                                                  2 - initial mode entry, setup things and paint the screen
//
  uint8_t modeHandler_Home(uint8_t forceRefresh);
  uint8_t modeHandler_Manual(uint8_t forceRefresh);
  uint8_t modeHandler_Status(uint8_t forceRefresh);
  uint8_t modeHandler_Setup(uint8_t forceRefresh);

  uint8_t callHandler(uint8_t needs_refresh);

};

extern OSLocalUI localUI;

#endif

