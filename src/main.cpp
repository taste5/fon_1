#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "buttons.h"
#include "ring.h"
#include "secrets.h"
#include "message.h"
#include "custom_keypad.h"

#ifdef ENABLE_WLED
#include "wled_controller.h"
WLEDController fixtures[WLED_NUM_FIXTURES] = 
{
 {WLED_IP_OBI},
 {WLED_IP_PIX},
 {WLED_IP_MIN},
};
#endif
byte rowPins[KEY_ROWS] = {KEY_PIN_ROW_0,KEY_PIN_ROW_1,KEY_PIN_ROW_2,KEY_PIN_ROW_3}; //connect to the row pinouts of the kpd
byte colPins[KEY_COLS] = {KEY_PIN_COL_0, KEY_PIN_COL_1,KEY_PIN_COL_2}; //connect to the column pinouts of the kpd


hw_timer_t *timer = NULL;
Melody melody;
Keypad kpd = Keypad(makeKeymap(keys),rowPins,colPins,(byte)KEY_ROWS,(byte)KEY_COLS);
Button pickup;
Button modifiers[MODIFIER_CNT];
SystemData MachineData;
OSCHandler Osc(LOCAL_PORT,REMOTE_PORT,REMOTE_IP);
MusicalData keypadNotes(notesForKeys, KEY_ROWS * KEY_COLS);




byte getCurrentState(){
  return MachineData.state;
}

void keypadEvent(KeypadEvent key){
  MachineData.event = EVENT_KEY_PRESSED;
}

void IRAM_ATTR timerEvent() {
  MachineData.event = EVENT_TIMER;
}



void blinkBuiltin(int cycle){
  
  static unsigned long lastBlink=0;
  if(millis() - lastBlink > cycle){digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));lastBlink = millis();}
}

void onEnter(enum States s)
{
  switch (s)
  {
    case STATE_NOT_CONNECTED:
      digitalWrite(LED_BUILTIN, HIGH);
    break;

    case STATE_PICKEDUP:
      digitalWrite(LED_BUILTIN,LOW);
      if (MachineData.prev_state == STATE_IDLE)
      {
        // enable osc keypress send and tone feedback
        MachineData.keypadFlags |= (1<< KEYPAD_SEND) | (1<<KEYPAD_TONE);
      }else if (MachineData.prev_state == STATE_RINGING)
      {
        //enable Midi masking
        MachineData.keypadFlags |= (1<< KEYPAD_MIDI);
      }
    break;
    case STATE_CONFIG:
      MachineData.history.clear();
    break;
  }
}

void onExit(enum States s)
{
  switch (s)
  {
  case STATE_NOT_CONNECTED:
    digitalWrite(LED_BUILTIN,LOW);
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
    Osc.send("/timer",timerAlarmEnabled(timer));
    MachineData.history.clear();
      
    break;
  default:
    break;
  }
}


void transitToState(enum States s)
{
  Serial.print("Transition from ");Serial.print(MachineData.state); Serial.print(" to ");Serial.println(s);
  Osc.send("/state", s);
  if (s >= STATE_CNT)
  {
    Serial.print("Invalid Call to transitToState: ");
    Serial.println(s);
    char buffer[64];
    snprintf(buffer,sizeof(buffer),"invalid input: %i", s);
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
    if(
      (reading == BTN_RELEASED && 
        (MachineData.state == STATE_IDLE || MachineData.state == STATE_RINGING)) ||
      (reading == BTN_PRESSED && MachineData.state == STATE_PICKEDUP)
    )
    {
      MachineData.event = EVENT_PICKUP;
    }
}

void modifierKeypresEvent()
{
  for (int i = 0; i < MODIFIER_CNT; i++)
  {
    if (checkBtn(&modifiers[i]))
    {
      MachineData.modifier_active ^= (1<<i);
      Osc.send("/log/modifier",i, MachineData.modifier_active);
      MachineData.event = EVENT_MODIFER;
    }

  }
}

void processKeyPressOnPickedUp(){
  for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
  {
    if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
    {
      switch (kpd.key[i].kstate) 
      {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
        case PRESSED:
        Serial.println("key");
          if(MachineData.keypadFlags & (1<<KEYPAD_TONE)) keypadNotes.playNote( kpd.key[i].kchar);
          if(MachineData.keypadFlags & (1<<KEYPAD_SEND)) Osc.sendChar("/key", kpd.key[i].kchar);
          if(MachineData.keypadFlags & (1<<KEYPAD_MIDI)) Osc.send("/key/note", keypadNotes.getMidiNote(kpd.key[i].kchar, MachineData.modifier_active), 0x7F);
        break;
        case RELEASED:
          if(MachineData.keypadFlags & (1<<KEYPAD_MIDI))Osc.send("/key/note", keypadNotes.getMidiNote(kpd.key[i].kchar, MachineData.modifier_active), 0x00);
        break;
      }
    }
  }
}

void  processKeypressOnConfig(){
  for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
  {
    if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
    {
      switch (kpd.key[i].kstate) 
      {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
        case PRESSED:
         MachineData.history.push(kpd.key[i].kchar);
        break;
        default:
        break;
      }
    }
  }
}


void processEvent(){
  byte event = EVENT_NONE;
  byte counter = 0;
  byte storage;
  if(MachineData.event){
    cli();

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
        // processIncomingMessage();
        break;
      
      case EVENT_KEY_PRESSED:
        // processKeyPressOnIdle();
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
      case EVENT_MODIFER:
        transitToState(STATE_CONFIG);
      default:
      blinkBuiltin(500);
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
          transitToState(STATE_PICKEDUP);
        break;
      
        case EVENT_LOST_CONNECTION:
          // goToState(STATE_NOT_CONNECTED);
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
          processKeyPressOnPickedUp();
        break;

        case EVENT_PICKUP:
          transitToState(STATE_IDLE);
        break;
      
        case EVENT_LOST_CONNECTION:
          // transitToState(STATE_NOT_CONNECTED);
        break;

        default:
        break;
      }
    break;
    case STATE_CONFIG:
      switch (event)
      {
      case EVENT_MODIFER:
        transitToState(STATE_IDLE);
        break;
      case EVENT_KEY_PRESSED:
        processKeypressOnConfig();
      break;
      
      default:
        if (MachineData.history.isFull(3))
        {
          tone(BUZZER_PIN,440,500);
          Serial.printf("to int:%i\n", MachineData.history.toInt());
          MachineData.timer_cycle_time_s = MachineData.history.toInt();
          timerAlarmWrite(timer,S_TO_US(MachineData.timer_cycle_time_s),false);
          MachineData.history.clear();
        }
      break;
      }
    break;
  }
}



void setup(){

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  btnInit(&pickup,PICKUP_PIN, 0);
  const int modifierpins[] = {MODIFIER_KEY_LEFT,MODIFIER_KEY_RIGHT};
  for (int i = 0; i < MODIFIER_CNT; i++)
  {
    btnInit(&modifiers[i], modifierpins[i],i+1);
  }
   


  Serial.begin(9600);
  delay(100);
  Osc.attachStateTransitionCallback(transitToState);
  kpd.addEventListener(keypadEvent);
  
  MachineData.timer_cycle_time_s = 6;
  timer = timerBegin(0, TIMER_PRESCALER, true);  
  timerAttachInterrupt(timer, &timerEvent, true);
  timerAlarmWrite(timer, (uint64_t)(MachineData.timer_cycle_time_s * pow(10,6)), false);
  
  transitToState(STATE_NOT_CONNECTED);
}

void loop() {
  
  //Handle Events
  if (WiFi.status() == WL_CONNECTION_LOST)
  {
    MachineData.event = EVENT_LOST_CONNECTION;
  }

  kpd.getKeys();
  pickupEvent();
  modifierKeypresEvent();

   


  Osc.poll();


 processEvent();
}
