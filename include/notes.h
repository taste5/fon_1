#ifndef NOTES_H
#define NOTES_H

// Octave below A4 (A3 to G#4)
#define NOTE_A3_FREQ     220.00f
#define NOTE_AS3_FREQ    233.08f  // A#3/Bb3
#define NOTE_B3_FREQ     246.94f
#define NOTE_C4_FREQ     261.63f
#define NOTE_CS4_FREQ    277.18f  // C#4/Db4
#define NOTE_D4_FREQ     293.66f
#define NOTE_DS4_FREQ    311.13f  // D#4/Eb4
#define NOTE_E4_FREQ     329.63f
#define NOTE_F4_FREQ     349.23f
#define NOTE_FS4_FREQ    369.99f  // F#4/Gb4
#define NOTE_G4_FREQ     392.00f
#define NOTE_GS4_FREQ    415.30f  // G#4/Ab4

// Octave centered at A4 (A4 to G#5)
#define NOTE_A4_FREQ     440.00f
#define NOTE_AS4_FREQ    466.16f  // A#4/Bb4
#define NOTE_B4_FREQ     493.88f
#define NOTE_C5_FREQ     523.25f
#define NOTE_CS5_FREQ    554.37f  // C#5/Db5
#define NOTE_D5_FREQ     587.33f
#define NOTE_DS5_FREQ    622.25f  // D#5/Eb5
#define NOTE_E5_FREQ     659.25f
#define NOTE_F5_FREQ     698.46f
#define NOTE_FS5_FREQ    739.99f  // F#5/Gb5
#define NOTE_G5_FREQ     783.99f
#define NOTE_GS5_FREQ    830.61f  // G#5/Ab5

 struct MusicalDataInterface
{
    float note;
    int dur;
    float pitch;
};



#endif