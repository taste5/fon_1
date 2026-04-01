#ifndef __APP_FSM_H__
#define __APP_FSM_H__

#include "globals.h"
#include "message.h"
#include "custom_keypad.h"
#include "buttons.h"
#include "ring.h"

extern Button modifiers[MODIFIER_CNT];
extern Button pickup;
extern Melody melody;
extern hw_timer_t *timer;
extern SystemData MachineData;
extern OSCHandler Osc;
extern MusicalData keypadNotes;

#ifdef ENABLE_WLED
#include "wled_controller.h"
extern WLEDManager wledManager;
#endif

void transitToState(enum States s);
void pickupEvent();
void modifierKeyPressEvent();
void processEvent();

#endif
