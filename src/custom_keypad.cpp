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


// Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, KEY_ROWS, KEY_COLS );

MusicalData::MusicalData(const float *noteData, byte arrLen)
    : noteData(noteData), noteArrayLen(arrLen)
{}


uint8_t MusicalData::calculatePos(char k)
{
    uint8_t ret = 1;
    if (isDigit(k))
    {
        ret = k - '0';
    }else if (k == '*')
    {
        ret = ':' - '0'; //char after 9
    }
    else if (k == '#')
    {
        ret = ';' - '0'; //char after :
    }
    else 
    {
        Serial.println("MusicalData: Invalid input to Pos");
    }
    return ret;   
}

float MusicalData::playNote(char k){
   byte idx = this->calculatePos(k);
   float note = noteData[idx];
   Serial.printf("idx: %i\n", idx);
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

uint8_t MusicalData::getMidiNote(char k)
{

    return calculatePos(k) + 66; // F Sharp
}

void MusicalData::stopNoteAfterDur(){
    // if (noteTimer && noteTimer - millis() > dur)
    // {
    //     Serial.print("Stopped after dur");
    //     noteTimer = 0;
    //     noTone(BUZZER_PIN);
    //     digitalWrite(LED_BUILTIN,LOW);
    // }
    
}

// void loop() {
//     loopCount++;
//     if ( (millis()-startTime)>5000 ) {
//         Serial.print("Average loops per second = ");
//         Serial.println(loopCount/5);
//         startTime = millis();
//         loopCount = 0;
//     }

//     // Fills kpd.key[ ] array with up-to 10 active keys.
//     // Returns true if there are ANY active keys.
//     if (kpd.getKeys())
//     {
//         for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
//         {
//             if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
//             {
//                 switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
//                     case PRESSED:
//                     msg = " PRESSED.";
//                 break;
//                     case HOLD:
//                     msg = " HOLD.";
//                 break;
//                     case RELEASED:
//                     msg = " RELEASED.";
//                 break;
//                     case IDLE:
//                     msg = " IDLE.";
//                 }
//                 Serial.print("Key ");
//                 Serial.print(kpd.key[i].kchar);
//                 Serial.println(msg);
//             }
//         }
//     }
// }  // End loop