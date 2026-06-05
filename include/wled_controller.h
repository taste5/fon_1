#ifndef WLED_CONTROLLER_H
#define WLED_CONTROLLER_H

#include "config.h"

#ifdef ENABLE_WLED

#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WLED_IP_SIZE 16
#define WLED_URL_SIZE 64
#define WLED_PAYLOAD_SIZE 128
#define WLED_NUM_FIXTURES WLED_N



class WLEDController {
private:
    HTTPClient http;
    char wledIP[WLED_IP_SIZE];
    char url[WLED_URL_SIZE];
    char payload[WLED_PAYLOAD_SIZE];
    void httpTask();
        // Static wrapper for task
    static void httpTaskWrapper(void *parameter) {
        WLEDController *instance = (WLEDController*)parameter;
        instance->httpTask();
        vTaskDelete(NULL);
    };
   
public:
    WLEDController(const char* ip);
   
    void setPreset(int presetNum);
    void setState(bool on);
    void setByIndex(int idx);
    void sendCommand();
};

class WLEDManager
{
private:
    WLEDController* wled;
    int numControllers;
    
    
public:
    WLEDManager(WLEDController* _wled, int _n);
    void setByIndex(int idx);
};




#endif
#endif
