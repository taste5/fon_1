#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "buttons.h"
#include "ring.h"
#include "secrets.h"
#include "message.h"
#include "custom_keypad.h"
#include "app_fsm.h"
#include "wifi_audio.h"

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
WiFiAudio wifiAudio;

byte getCurrentState() {
    return MachineData.state;
}

void setAudioEnabled(bool enabled) {
    if (enabled) wifiAudio.begin();
    else         wifiAudio.stop();
}

bool getAudioEnabled() {
    return wifiAudio.isEnabled();
}

void setAudioUrl(const char* url) {
    wifiAudio.setUrl(url);
}

const char* getAudioUrl() {
    return wifiAudio.getUrl();
}

void setSleepAllowed(bool allowed) {
    MachineData.sleepAllowed = allowed;
    setCpuFrequencyMhz(allowed ? 80 : 240);
    WiFi.setSleep(allowed && MachineData.state == STATE_IDLE);
}

void setBeep(bool yesno) {
    MachineData.beep_on = yesno ? (int8_t)1 : (int8_t)0;
}

void keypadEvent(KeypadEvent key) {
    MachineData.event = EVENT_KEY_PRESSED;
}

void IRAM_ATTR timerEvent() {
    MachineData.event = EVENT_TIMER;
}

void beep(SystemData *d) {
    const int yes_freq[] = {440, 880};
    const int no_freq[] = {880, 440};
    static unsigned long last_beep = 0;
    static int last_freq_idx = 0;
    const unsigned long now = millis();
    if (now - last_beep > 100) {
        last_beep = now;
        const int hz = (d->beep_on) ? yes_freq[last_freq_idx] : no_freq[last_freq_idx];
        tone(BUZZER_PIN, hz, 100);
        if (last_freq_idx == 1) {
            d->beep_on = -1;
            last_freq_idx = 0;
            return;
        }
        last_freq_idx++;
    }
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
    modifierKeyPressEvent();

    Osc.poll();
    processEvent();
    if (MachineData.beep_on != -1) {
        beep(&MachineData);
    }
    delay(10);
}
