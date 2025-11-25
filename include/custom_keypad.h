#ifndef __CUSTOMKEYPAD_H__
#define __CUSTOMKEYPAD_H__


#include <Arduino.h>
#include <Keypad.h>
#include "globals.h"
#include "config.h"
#include "notes.h"

#define STAR_CODE 10
#define HASH_CODE 11

#define KEY_ROWS 4
#define KEY_COLS 3

#define KEY_PRESS_TONE_DUR 400

#define MODIFIER_OFFSET 11

extern const float notesForKeys[KEY_ROWS * KEY_COLS];
extern char keys[KEY_ROWS][KEY_COLS];
class MusicalData
{
    public:
    MusicalData(const float * keypadNoteData, byte arrLen);
    float playNote(char k);
    uint8_t getMidiNote(char k, byte offset);
    private:
    byte calculatePos(char k);
    const float *noteData;
    int dur = KEY_PRESS_TONE_DUR;
    int noteArrayLen = KEY_ROWS * KEY_COLS;
    uint64_t noteTimer = 0;

};





#endif 