#include "buttons.h"

int checkBtn(Button *btn){
   if (digitalRead(btn->pin) != btn->state && millis()- btn->lastDbT > DEBOUNCE_T)
    {
        // physical state can be 0|1, logical state can be NONE=0,PRESS=switch from high to low, RELEASE,DOUBLE
        int state_offset = 1;
        btn->state = !btn->state;
        
        if(btn->doubleclick_enabled && btn->state == BTN_PRESSED)
        {
            //start dc timeout in case it is not.
            if(millis() - btn->doubleclick_timeout > DOUBLECLICK_T)
            {
                btn->doubleclick_timeout = millis();
            }
            //register double click and reset clock.
            else
            {
                state_offset++;
                btn->doubleclick_timeout = 0;
            }
            
        }
        
        btn->lastDbT = millis();
        return btn->state + state_offset;
    }
    return BTN_NONE;
}

void btnInit(Button *btn,int pin, int id){
    btn->pin = pin;
    pinMode(btn->pin, INPUT_PULLUP);
    btn->state = digitalRead(btn->pin);
    btn->lastDbT = millis();
    btn->id = id;
}