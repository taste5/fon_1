#include "ring.h"



bool playChord(const int chord[], Melody *m) {
  const int size = 3;
  if (m->currentNote >= size)
  {
    return false;
  }
  
  if ( millis() - m->lastTone > m->noteDuration)
  {
    noTone(BUZZER_PIN);
    tone(BUZZER_PIN, chord[m->currentNote]);
    m->currentNote++;
    m->lastTone = millis();
  }
  return true;
}
void ring( Melody *m){
  const int *chords[] = {chordC,chordF,chordG};
    
switch (m->in_pause)
{
case 0:
if (m->currentChord >= 3)
{
  m->in_pause = true;
  noTone(BUZZER_PIN);
}

if(!playChord(chords[m->currentChord],m)){
    m->currentChord ++;
    m->currentNote = 0;
 }
  
  break;

case 1:
if (millis() - m->chordDelay > m->lastTone)
{
    m->lastTone = millis();
    m->in_pause = false;
    m->currentChord = 0;
}
default:
  break;
}
 
}

