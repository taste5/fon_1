#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define S_TO_US(s)  ((s) * 1000000UL)

#include <Arduino.h>


//TODO: Add to str function …
struct KeypadHistory
{
  /*
  static constexpr size_t BUFFER_SIZE = 16;
│      │         │      │             │
│      │         │      │             └── Value
│      │         │      └── Name (CAPS = convention for constants)
│      │         └── Type (unsigned integer for sizes)
│      └── Compile-time constant
└── Shared across all instances
*/
  static constexpr size_t BUFFER_SIZE = 10;
  char buffer[BUFFER_SIZE] = {'\0'};
  byte idx=0;
  byte cnt=0; //Track how many values are stored
   void push(char key) {
        if (idx < BUFFER_SIZE - 1) {  // Leave room for '\0'
            buffer[idx++] = key;
            buffer[idx] = '\0';
        }
    }

  int toInt() const
  {
    char buf[BUFFER_SIZE];
    int j=0;
    for (int i = 0; i < idx; i++)
    {
      char c = buffer[i];
      if (isDigit(c))
      {
        buf[j++] = c;
      }
    }
    buf[j] = '\0';
    return atoi(buf);
  }
  void clear(){
    buffer[0]='\0';
    idx = 0;
  }
  bool isFull(uint8_t limit = 0) const
  {
      uint8_t maxIdx = (limit > 0) ? limit : BUFFER_SIZE - 1;
      return idx >= maxIdx;
  }
};
  
struct SystemData {
  byte state;
  byte prev_state;
  volatile byte event;
  byte keypadFlags;
  float timer_cycle_time_s;
  byte modifier_active;
  KeypadHistory history;
};

enum ModifierFlags {
  MODIFIER_BIT_LEFT,
  MODIFIER_BIT_RIGHT,
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
  STATE_CONFIG = 0x08,
  STATE_CNT = 9,
};


enum Events {
  EVENT_NONE = 0x00,
  EVENT_KEY_PRESSED = 0x01,
  EVENT_INCOMING_MESSAGE = 0x02,
  EVENT_PICKUP = 0x04,
  EVENT_TIMER = 0x08,
  EVENT_MODIFER = 0x10,
  EVENT_LOST_CONNECTION = 0x20,
};


byte getCurrentState();
#endif 