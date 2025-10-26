#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "buttons.h"
#include "chords.h"
#include <WiFi.h>



// ===== WiFi settings =====
const char* ssid     = "netzmork2G";
const char* password = "floodzone";

// ===== Access Point fallback =====
const char* ap_ssid = "ESP32_OSC_AP";
const char* ap_pass = "12345678";



// Chord definitions


struct Melody {
  int noteDuration = 100; // ms per note in arpeggio
  int currentNote = 0;
  int currentChord =0;
  int chordDelay = 400;   // pause after each chord
  unsigned long lastTone;
  bool in_pause = false;
  int ring_twice = 0;

};

struct SystemData {
  bool is_ringing = false;
};


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







void setup(){

  pinMode(PICKUP_PIN, INPUT_PULLUP);
  pickup.pin = PICKUP_PIN;
  pickup.state = digitalRead(pickup.pin);
  pickup.lastDbT = millis();
  pickup.id = 0;
  data.is_ringing = true;

  Serial.begin(9600);
    delay(1000);


  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();

  // Try to connect for 10 seconds
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect. Starting Access Point...");
    WiFi.softAP(ap_ssid, ap_pass);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
  }

  Udp.begin(LOCAL_PORT);
  Serial.printf("Listening for OSC on port %d\n", LOCAL_PORT);
};




void loop() {

    // Receive OSC messages
  OSCMessage msgIN;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      msgIN.fill(Udp.read());
    }
    if (!msgIN.hasError()) {
      msgIN.dispatch("/test", handleTest);
      msgIN.dispatch("/ring", handleRing);
    }
  }

  // Example: Send an OSC message every 5 seconds
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 5000) {
    sendHeartbeat();
    lastSend = millis();
  }




  int btn_state = checkBtn((&pickup));
    switch (btn_state)
    {
    
    case BTN_PRESSED:
    break;
    case BTN_RELEASED:
    data.is_ringing = false;
    noTone(BUZZER_PIN);
    break;
    default:
    break;
  }
  if (data.is_ringing)
  {
    ring(&melody);
  }
  



}
