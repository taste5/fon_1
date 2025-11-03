#ifndef __MESSAGE_H__
#define __MESSAGE_H__


#include <Arduino.h>
#include <OSCMessage.h>
#include "globals.h"
#include "config.h"
#include <WiFiUdp.h>
#include <WiFi.h>

typedef void (*StateTransitionCallback)(enum States newState);


class OSCHandler{
    public:
        OSCHandler();
        void begin();

        void send(const char *address, int val);
        void send(const char *address, float val);
        void send(const char *address, const char *val);
        void attachStateTransitionCallback(StateTransitionCallback cb);
        static OSCHandler* instance;

        void poll();
    
    private:
        static StateTransitionCallback stateCallback;
        static void handleStateCmd(OSCMessage &msg, int addrOffset);
        static void handleConfigCmd(OSCMessage &msg, int addrOffset);
        void _send(const char *addr,void *arg, char type);
        void debug(OSCMessage &msg);
        void composeMsg(OSCMessage &msg, const char *addr);
        void transmitMsg(OSCMessage &msg);
        WiFiUDP udp;
        SystemData *sysData;
        const char * remoteIp;
        const unsigned int localPort;
        const unsigned int remotePort;

};



wl_status_t connectToWifi(const char * ssid, const char *password, const char *ap_ssid, const char *ap_pass);

#endif 