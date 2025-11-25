#ifndef __RING_H__
#define __RING_H__


#include <Arduino.h>
#include "globals.h"
#include "config.h"


const int chordC[3] = {523, 659, 784};     // C major
const int chordF[3] = {698, 880, 1046};    // F major
const int chordG[3] = {784, 988, 1175};    // G major
const unsigned long ringPattern[] = { 170, 170, 460 }; // ms durations

struct Melody {
  int noteDuration = 100; // ms per note in arpeggio
  int currentNote = 0;
  int currentChord =0;
  int chordDelay = 400;   // pause after each chord
  unsigned long lastTone;
  bool in_pause = false;
  int ring_twice = 0;
  
};
void ring( Melody *m);
#endif 