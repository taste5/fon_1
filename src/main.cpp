#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "buttons.h"
#include "ring.h"
#include "secrets.h"
#include "message.h"

Melody melody;
WiFiUDP Udp;
Button pickup;
SystemData MachineData;

void blinkBuiltin(){
  const int cycleTime = 500;
  static unsigned long lastBlink=0;
  if(millis() - lastBlink > cycleTime){digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));lastBlink = millis();}
}


void onExit(enum States s)
{
  switch (s)
  {
  case STATE_NOT_CONNECTED:
    digitalWrite(LED_BUILTIN,LOW);
    break;
  
  default:
    break;
  }
}

void onEnter(enum States s)
{
  switch (s)
  {
  case STATE_NOT_CONNECTED:
    digitalWrite(LED_BUILTIN, HIGH);
    break;
  
  default:
    break;
  }
}

void transitToState(enum States s)
{
  Serial.print("Transition from ");Serial.print(MachineData.state); Serial.print(" to ");Serial.println(s);
  // osc.sendCurrentStatus();
  onExit((enum States)MachineData.state);
  onEnter(s);
  MachineData.state = s;
}
void processEvent(){
  byte event = EVENT_NONE;
  byte counter = 0;
  byte storage;

  do{
    storage =1<<(counter);  /* Maske zum erkennen von Bits erzeugen */
    if ((storage&MachineData.event)!=0)
    {
      event = storage; //Event recognised, extract event
      MachineData.event &=~storage; // clear from memory
    }
    counter++;
  }while (
    (counter < (sizeof(MachineData.event)*8))&&
    (event == EVENT_NONE)
  );


  switch (MachineData.state)
  {
    case STATE_NOT_CONNECTED:
      MachineData.connectionStatus = connectToWifi(ssid,password,ap_ssid,ap_pass);
      if (MachineData.connectionStatus == WL_CONNECTED)
      {
        transitToState(STATE_IDLE);
      }
      
    break;
    case STATE_IDLE:
      switch (event)
      {
      case EVENT_INCOMING_MESSAGE:
        // processIncomingMessage();
        break;
      
      case EVENT_KEY_PRESSED:
        // processKeyPressOnIdle;
      break;

      case EVENT_PICKUP:
        // goToState(STATE_PICKUP); //
      break;

      case EVENT_LOST_CONNECTION:
        transitToState(STATE_NOT_CONNECTED);
      break;
      default:
      blinkBuiltin();
        break;
      }
    break;
      case STATE_RINGING:
      switch (event)
      {
        case EVENT_INCOMING_MESSAGE:
          // processIncomingMessage();
        break;
      
        case EVENT_KEY_PRESSED:
          // processKeyPressOnRing;
        break;

        case EVENT_PICKUP:
          // goToState(STATE_PICKUP);
        break;
      
        case EVENT_LOST_CONNECTION:
          // goToState(STATE_NOT_CONNECTED);
        break;

        default:
          // ring();
        break;
      }
    break;
    case STATE_PICKEDUP:
       switch (event)
      {
        case EVENT_INCOMING_MESSAGE:
          // processIncomingMessage();
        break;
      
        case EVENT_KEY_PRESSED:
          // processKeyPressOnPickup;
        break;

        case EVENT_PICKUP:
          // goToState(STATE_IDLE);
        break;
      
        case EVENT_LOST_CONNECTION:
          // goToState(STATE_NOT_CONNECTED);
        break;

        default:
        break;
      }
    break;
      default:
    break;
  }
}


void setup(){

  pinMode(LED_BUILTIN, OUTPUT);
  btnInit(&pickup,PICKUP_PIN, 0);
  Serial.begin(9600);
  delay(100);

  transitToState(STATE_NOT_CONNECTED);

  

}

void loop() {
  if (WiFi.status() == WL_CONNECTION_LOST)
  {
    MachineData.event = EVENT_LOST_CONNECTION;
  }
  
  // Receive OSC messages
  OSCMessage msgIN;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      msgIN.fill(Udp.read());
    }
    if (!msgIN.hasError()) {
      // msgIN.dispatch("/test", handleTest);
      // msgIN.dispatch("/ring", handleRing);
    }
  }

  // Example: Send an OSC message every 5 seconds
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 5000) {
    // sendHeartbeat();
    lastSend = millis();
  }




  int btn_state = checkBtn((&pickup));
    switch (btn_state)
    {
    
    case BTN_PRESSED:
    break;
    case BTN_RELEASED:
    noTone(BUZZER_PIN);
    break;
    default:
    break;
  }



  processEvent();
  


}
