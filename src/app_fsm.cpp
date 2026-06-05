#include "app_fsm.h"
#include "config.h"
#include <Arduino.h>

static void blinkBuiltin(int cycle) {
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > (unsigned long)cycle) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        lastBlink = millis();
    }
}

static void onEnter(enum States s)
{
    WiFi.setSleep(s == STATE_IDLE && MachineData.sleepAllowed);

    switch (s)
    {
    case STATE_NOT_CONNECTED:
        digitalWrite(LED_BUILTIN, HIGH);
        break;

    case STATE_PICKEDUP:
        digitalWrite(LED_BUILTIN, LOW);
        if (MachineData.prev_state == STATE_IDLE)
        {
            MachineData.keypadFlags |= (1 << KEYPAD_SEND) | (1 << KEYPAD_TONE);
        }
        else if (MachineData.prev_state == STATE_RINGING)
        {
            MachineData.keypadFlags |= (1 << KEYPAD_MIDI);
        }
        break;

    case STATE_CONFIG:
        MachineData.history.clear();
        break;

    default:
        break;
    }
}

static void onExit(enum States s)
{
    switch (s)
    {
    case STATE_NOT_CONNECTED:
        digitalWrite(LED_BUILTIN, LOW);
        break;

    case STATE_PICKEDUP:
        MachineData.keypadFlags = 0;
        break;

    case STATE_RINGING:
        noTone(BUZZER_PIN);
        break;

    case STATE_CONFIG:
        timerRestart(timer);
        timerAlarmEnable(timer);
        Osc.send("/timer/enabled", timerAlarmEnabled(timer));
        MachineData.history.clear();
        break;

    default:
        break;
    }
}

void transitToState(enum States s)
{
    Serial.print("Transition from "); Serial.print(MachineData.state); Serial.print(" to "); Serial.println(s);
    Osc.send("/state", s);

    if (s >= STATE_MAX)
    {
        Serial.print("Invalid Call to transitToState: ");
        Serial.println(s);
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "invalid input: %i", s);
        Osc.send("/error/state", buffer);
        return;
    }

    onExit((enum States)MachineData.state);
    if (MachineData.state != STATE_CONFIG) MachineData.prev_state = MachineData.state;
    onEnter(s);
    MachineData.state = s;
}

void pickupEvent()
{
    int reading = checkBtn(&pickup);
    if (
        (reading == BTN_RELEASED &&
         (MachineData.state == STATE_IDLE || MachineData.state == STATE_RINGING)) ||
        (reading == BTN_PRESSED && MachineData.state == STATE_PICKEDUP)
    )
    {
        MachineData.event = EVENT_PICKUP;
    }
}

void modifierKeyPressEvent()
{
    for (int i = 0; i < MODIFIER_CNT; i++)
    {
        if (checkBtn(&modifiers[i]))
        {
            MachineData.modifier_active ^= (1 << i);
            Osc.send("/log/modifier", i, MachineData.modifier_active);
            MachineData.event = EVENT_MODIFIER;
        }
    }
}

// ---------- key handlers ----------

static void keyPressOnPickedUp(char key, KeyState state) {
    if (state == PRESSED) {
        if (MachineData.keypadFlags & (1 << KEYPAD_TONE)) keypadNotes.playNote(key);
        if (MachineData.keypadFlags & (1 << KEYPAD_SEND)) Osc.sendChar("/key", key);
        if (MachineData.keypadFlags & (1 << KEYPAD_MIDI)) {
            Osc.send("/key/note", keypadNotes.getMidiNote(key, MachineData.modifier_active), 0x7F);
        }
    } else {
        if (MachineData.keypadFlags & (1 << KEYPAD_MIDI)) {
            Osc.send("/key/note", keypadNotes.getMidiNote(key, MachineData.modifier_active), 0x00);
        }
    }
}

static void keyPressOnIdle(char key, KeyState state)
{
    if (state == PRESSED)
    {
        int idx = keypadNotes.calculatePos(key);
        Serial.printf("index: %i\n", idx);
#ifdef ENABLE_WLED
        wledManager.setByIndex(idx);
#endif
    }
}

static void keypressOnConfig(char key, KeyState state) {
    if (state == PRESSED) {
        MachineData.history.push(key);
    }
}

// ---------- event loop ----------

void processEvent()
{
    byte event = EVENT_NONE;
    byte counter = 0;
    byte storage;

    if (MachineData.event) {
        cli();
        do {
            storage = 1 << (counter);
            if ((storage & MachineData.event) != 0)
            {
                event = storage;
                MachineData.event &= ~storage;
            }
            counter++;
        } while (
            (counter < (sizeof(MachineData.event) * 8)) &&
            (event == EVENT_NONE)
        );
    }
    sei();

    switch (MachineData.state)
    {
    case STATE_NOT_CONNECTED:
        Osc.begin();
        if (Osc.getConnectionState())
        {
            transitToState(STATE_IDLE);
        }
        break;

    case STATE_IDLE:
        switch (event)
        {
        case EVENT_INCOMING_MESSAGE:
            break;
        case EVENT_KEY_PRESSED:
            processKeys(keyPressOnIdle);
            break;
        case EVENT_PICKUP:
            transitToState(STATE_PICKEDUP);
            break;
        case EVENT_TIMER:
            transitToState(STATE_RINGING);
            break;
        case EVENT_LOST_CONNECTION:
            transitToState(STATE_NOT_CONNECTED);
            break;
        case EVENT_MODIFIER:
            transitToState(STATE_CONFIG);
            break;
        default:
            blinkBuiltin(500);
            break;
        }
        break;

    case STATE_RINGING:
        switch (event)
        {
        case EVENT_INCOMING_MESSAGE:
            break;
        case EVENT_KEY_PRESSED:
            break;
        case EVENT_PICKUP:
            transitToState(STATE_PICKEDUP);
            break;
        case EVENT_LOST_CONNECTION:
            break;
        default:
            ring(&melody);
            blinkBuiltin(100);
            break;
        }
        break;

    case STATE_PICKEDUP:
        switch (event)
        {
        case EVENT_KEY_PRESSED:
            processKeys(keyPressOnPickedUp, keyPressOnPickedUp);
            break;
        case EVENT_PICKUP:
            transitToState(STATE_IDLE);
            break;
        case EVENT_LOST_CONNECTION:
            break;
        default:
            break;
        }
        break;

    case STATE_CONFIG:
        switch (event)
        {
        case EVENT_MODIFIER:
            transitToState(STATE_IDLE);
            break;
        case EVENT_KEY_PRESSED:
            processKeys(keypressOnConfig);
            break;
        default:
            if (MachineData.history.isFull(3))
            {
                tone(BUZZER_PIN, 440, 500);
                Serial.printf("to int:%i\n", MachineData.history.toInt());
                MachineData.timer_cycle_time_s = MachineData.history.toInt();
                timerAlarmWrite(timer, S_TO_US(MachineData.timer_cycle_time_s), false);
                MachineData.history.clear();
            }
            break;
        }
        break;
    }
}
