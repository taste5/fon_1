#ifndef WLED_CONTROLLER_H
#define WLED_CONTROLLER_H

#include "config.h"

#ifdef ENABLE_WLED

#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WLED_IP_SIZE 16
#define WLED_URL_SIZE 64
#define WLED_PAYLOAD_SIZE 128
#define WLED_NUM_FIXTURES 3

// enum WLEDFixtures {
//     WLED_OBI,
//     WLED_PIX,
//     WLED_MIN
// };

class WLEDController {
private:
    HTTPClient http;
    char wledIP[WLED_IP_SIZE];
    char url[WLED_URL_SIZE];
    char payload[WLED_PAYLOAD_SIZE];
    
public:
    WLEDController(const char* ip);
    
    bool setPreset(int presetNum);
    bool setState(bool on);
    
private:
    bool sendCommand();
};

#endif
#endif
