#ifndef __CUSTOMKEYPAD_H__
#define __CUSTOMKEYPAD_H__


#include <Arduino.h>
#include <Keypad.h>
#include "globals.h"
#include "config.h"
#include "notes.h"

#define STAR_INDEX 9
#define HASH_INDEX 11
#define ZERO_INDEX 10

#define KEY_ROWS 4
#define KEY_COLS 3

#define KEY_PRESS_TONE_DUR 400

#define KEY_N_TOTAL 11


//callback function ptr for keypress readings
typedef void (*KeyHandler)(char key, KeyState state);
void processKeys(KeyHandler onPress, KeyHandler onRelease = nullptr);

// TODO: find out what is best to do here?
//defined in main.cpp
extern Keypad kpd;

//defined in custom_keypad.cpp
extern const float notesForKeys[KEY_ROWS * KEY_COLS];
extern char keys[KEY_ROWS][KEY_COLS];



class MusicalData
{
    public:
    MusicalData(const float * keypadNoteData, byte arrLen);
    float playNote(char k);
    uint8_t getMidiNote(char k, byte offset);
    byte calculatePos(char k);
    private:
    const float *noteData;
    int dur = KEY_PRESS_TONE_DUR;
    int noteArrayLen;

};





#endif 