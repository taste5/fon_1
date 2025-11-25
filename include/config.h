#ifndef __CONFIG_H__
#define __CONFIG_H__


#define BUZZER_PIN 23
#define PICKUP_PIN 22



#define KEY_PIN_COL_0 33
#define KEY_PIN_COL_1 25
#define KEY_PIN_COL_2 26

#define KEY_PIN_ROW_0 27
#define KEY_PIN_ROW_1 14
#define KEY_PIN_ROW_2 12
#define KEY_PIN_ROW_3 13

#define MODIFIER_KEY_LEFT 4
#define MODIFIER_KEY_RIGHT 16
#define MODIFIER_CNT 2


#define REMOTE_IP "192.168.8.133" 
#define LOCAL_PORT 9000    // Port to listen for incoming OSC
#define REMOTE_PORT 9001 // Remote host to send OSC

#define TIMER_PRESCALER 80

#endif 