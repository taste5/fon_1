#ifndef __GLOBALS_H__
#define __GLOBALS_H__


#include <Arduino.h>


struct SystemData {
  byte state;
  byte prev_state;
  volatile byte event;
  byte keypadFlags;
  float timer_cycle_time_s;
  byte modifier_active;
};

enum ModifierFlags {
  MODIFIER_LEFT_ACTIVE,
  MODIFIER_RIGHT_ACTIVE,
};



enum KeypadFlags {
  KEYPAD_TONE,
  KEYPAD_SEND,
  KEYPAD_MIDI
};


enum States {
  STATE_NOT_CONNECTED = 0x00,
  STATE_IDLE = 0x01,
  STATE_RINGING = 0x02,
  STATE_PICKEDUP = 0x04,
  STATE_CNT,
};


enum Events {
  EVENT_NONE = 0x00,
  EVENT_KEY_PRESSED = 0x01,
  EVENT_INCOMING_MESSAGE = 0x02,
  EVENT_PICKUP = 0x04,
  EVENT_TIMER = 0x08,
  EVENT_LOST_CONNECTION = 0x10,
};


byte getCurrentState();
#endif 