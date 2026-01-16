#ifndef __CONFIG_H__
#define __CONFIG_H__

#define ENABLE_WLED

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


#define REMOTE_IP "192.168.8.255" 
#define LOCAL_PORT 9000    // Port to listen for incoming OSC
#define REMOTE_PORT 9001 // Remote host to send OSC


#ifdef ENABLE_WLED
#define WLED_IP_OBI "192.168.8.164" 
#define WLED_IP_PIX "192.168.8.145"
#define WLED_IP_MIN "192.168.8.116"
#endif

#define TIMER_PRESCALER 80

#endif 