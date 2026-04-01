#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "buttons.h"
#include "ring.h"
#include "secrets.h"
#include "message.h"
#include "custom_keypad.h"
#include "app_fsm.h"

#ifdef ENABLE_WLED
#include "wled_controller.h"
WLEDController wled[WLED_NUM_FIXTURES] =
{
    {WLED_IP_OBI},
    {WLED_IP_PIX},
    {WLED_IP_MIN},
};
WLEDManager wledManager(wled, WLED_NUM_FIXTURES);
#endif

byte rowPins[KEY_ROWS] = {KEY_PIN_ROW_0, KEY_PIN_ROW_1, KEY_PIN_ROW_2, KEY_PIN_ROW_3};
byte colPins[KEY_COLS] = {KEY_PIN_COL_0, KEY_PIN_COL_1, KEY_PIN_COL_2};
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, (byte)KEY_ROWS, (byte)KEY_COLS);
Button modifiers[MODIFIER_CNT];
Button pickup;

hw_timer_t *timer = NULL;
Melody melody;
SystemData MachineData;
OSCHandler Osc(LOCAL_PORT, REMOTE_PORT, REMOTE_IP);
MusicalData keypadNotes(notesForKeys, KEY_ROWS * KEY_COLS);

byte getCurrentState() {
    return MachineData.state;
}

void keypadEvent(KeypadEvent key) {
    MachineData.event = EVENT_KEY_PRESSED;
}

void IRAM_ATTR timerEvent() {
    MachineData.event = EVENT_TIMER;
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    btnInit(&pickup, PICKUP_PIN, 0);
    const int modifierpins[] = {MODIFIER_KEY_LEFT, MODIFIER_KEY_RIGHT};
    for (int i = 0; i < MODIFIER_CNT; i++)
    {
        btnInit(&modifiers[i], modifierpins[i], i + 1);
    }

    Serial.begin(9600);
    delay(100);
    Osc.attachStateTransitionCallback(transitToState);
    kpd.addEventListener(keypadEvent);

    MachineData.timer_cycle_time_s = 6;
    timer = timerBegin(0, TIMER_PRESCALER, true);
    timerAttachInterrupt(timer, &timerEvent, true);
    timerAlarmWrite(timer, S_TO_US(MachineData.timer_cycle_time_s), false);

    transitToState(STATE_NOT_CONNECTED);
}

void loop() {
    if (WiFi.status() == WL_CONNECTION_LOST)
    {
        MachineData.event = EVENT_LOST_CONNECTION;
    }

    kpd.getKeys();
    pickupEvent();
    modifierKeypresEvent();

    Osc.poll();
    processEvent();
}
