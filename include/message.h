#ifndef __MESSAGE_H__
#define __MESSAGE_H__


#include <Arduino.h>
#include <OSCMessage.h>
#include "globals.h"
#include "config.h"
#include <WiFiUdp.h>
#include <WiFi.h>
#include "secrets.h"

typedef void (*StateTransitionCallback)(enum States newState);

enum OSCConnectionStates{
    NOT_CONNECTED,
    CONNECTED,
    AP_OPEN
};

typedef class OSCPing{
    public:
    OSCPing();
    void begin(float cycleT_sec){resetTimer();this->cycleT_sec = cycleT_sec;};
    bool is_due(){int ret =false;ret = cycleT_sec?  millis()-lastPingT >(uint64_t)(cycleT_sec *1000) : false;return ret;};
    const char * getStrVal(){return strVal;};
    const char * getStrAnswer(){return strAnswer;};
    float getCycleT(){return cycleT_sec;};
    void setCycleT_sec(float t){cycleT_sec = t;};
    void resetTimer(){lastPingT = millis();};
    private:
    const char *strVal = "/ping";
    const char *strAnswer = "/pong";
    uint64_t lastPingT;
    float cycleT_sec = 60;
}OSCPing;

class OSCHandler{
    public:
        OSCHandler(unsigned int localPort, unsigned int remotePort, const char *remoteIp);
        void begin();

        void send(const char *address, int val);
        void sendChar(const char *address, char c);
        void send(const char *address, float val);
        void send(const char *address, const char *val);
        void send(const char *address, uint8_t, uint8_t);
        void attachStateTransitionCallback(StateTransitionCallback cb);
        static OSCHandler* instance;
        enum OSCConnectionStates getConnectionState(){return connectionState;};
        OSCPing ping;
        void poll();
    
    private:
        void connectToWifi();
        static StateTransitionCallback stateCallback;
        static void handleStateCmd(OSCMessage &msg, int addrOffset);
        static void handleConfigCmd(OSCMessage &msg, int addrOffset);
        static void handlePing(OSCMessage &msg, int addrOffset);
        void debug(OSCMessage &msg);
        void composeMsg(OSCMessage &msg, const char *addr);
        void transmitMsg(OSCMessage &msg);
        WiFiUDP udp;
        enum OSCConnectionStates connectionState;
        IPAddress remoteIp;
        IPAddress makeIP(const char * ipstr);
        const unsigned int localPort;
        const unsigned int remotePort;

};



wl_status_t connectToWifi(const char * ssid, const char *password, const char *ap_ssid, const char *ap_pass);

#endif 