#include "custom_keypad.h"


char keys[KEY_ROWS][KEY_COLS] = {
{'1','2','3'},
{'4','5','6'},
{'7','8','9'},
{'*','0','#'}
};

const float notesForKeys[KEY_ROWS * KEY_COLS] = {
     NOTE_G4_FREQ /*<-key 0 E-min*/, 
     NOTE_C4_FREQ, NOTE_E4_FREQ, NOTE_G4_FREQ ,     // C Major
     NOTE_G4_FREQ, NOTE_B4_FREQ, NOTE_D5_FREQ ,     // G Major
     NOTE_A4_FREQ, NOTE_C5_FREQ, NOTE_E5_FREQ ,     // A Minor
     NOTE_E4_FREQ, /*Key 0  */   NOTE_B4_FREQ ,     // E Minor
};




void processKeys(KeyHandler onPress, KeyHandler onRelease) 
{
    for (int i = 0; i < LIST_MAX; i++) {
        if (kpd.key[i].stateChanged) {
            switch (kpd.key[i].kstate) {
                case PRESSED:
                    if (onPress) onPress(kpd.key[i].kchar, PRESSED);
                    break;
                case RELEASED:
                    if (onRelease) onRelease(kpd.key[i].kchar, RELEASED);
                    break;
            }
        }
    }
}



MusicalData::MusicalData(const float *noteData, byte arrLen)
    : noteData(noteData), noteArrayLen(arrLen)
{}


uint8_t MusicalData::calculatePos(char k)
{
    uint8_t ret = 1;
    if (isDigit(k))
    {
        return k - '0';
    }else if (k == '*')
    {
       return STAR_CODE;
    }
    else if (k == '#')
    {
       return HASH_CODE;
    }
    else 
    {
        Serial.println("MusicalData: Invalid input to Pos");
        return 1;   
    }
}

float MusicalData::playNote(char k){
   byte idx = this->calculatePos(k);
   float note = noteData[idx];
   if(idx>=noteArrayLen)
   {
       tone(BUZZER_PIN,880);
       delay(5000);
       noTone(BUZZER_PIN);
       return 880;
    }  
   
    tone(BUZZER_PIN, noteData[idx], dur);
    return this->noteData[idx];
}

uint8_t MusicalData::getMidiNote(char k, byte offset_factor)
{

    return calculatePos(k) + offset_factor * MODIFIER_OFFSET + 66; // F Sharp
}