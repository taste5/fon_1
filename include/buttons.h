#ifndef   __BUTTONS_H__
#define __BUTTONS_H__

#include <Arduino.h>

#define DEBOUNCE_T      250
#define DOUBLECLICK_T   0xFFFF

typedef struct 
{
    uint8_t pin;
    uint8_t id;
    bool state;
    bool doubleclick_enabled;
    uint64_t lastDbT;
    uint64_t doubleclick_timeout;
}Button;

typedef enum 
{
    BTN_NONE,
    BTN_PRESSED,
    BTN_RELEASED,
    BTN_DOUBLECLICK
}BtnState;

int checkBtn(Button *btn);
#endif