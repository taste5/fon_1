#include "wled_controller.h"
#include "custom_keypad.h"
#ifdef ENABLE_WLED

WLEDController::WLEDController(const char* ip) {
    strncpy(wledIP, ip, WLED_IP_SIZE - 1);
    wledIP[WLED_IP_SIZE - 1] = '\0';
}

bool WLEDController::setPreset(int presetNum) {
    snprintf(payload, sizeof(payload), "{\"on\":true,\"ps\":%d}", presetNum);
    return sendCommand();
}

bool WLEDController::setState(bool on) {
    snprintf(payload, sizeof(payload), "{\"on\":%s}", on ? "true" : "false");
    return sendCommand();
}

bool WLEDController::sendCommand() {
    snprintf(url, sizeof(url), "http://%s/json/state", wledIP);
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    int httpCode = http.POST((uint8_t*)payload, strlen(payload));
    http.end();
    #ifdef WLED_DEBUG
    Serial.println("=== sendCommand Debug ===");
    Serial.printf("Target IP: %s\n", wledIP);
    Serial.printf("Payload: %s\n", payload);  // or however you build it
    
    
    Serial.println("======================");
    #endif
    
    return (httpCode == 200);
}

WLEDManager::WLEDManager(WLEDController* _wled, int _n)
:
wled(_wled),
numControllers(_n)
{
    if(WLED_N < numControllers)
    {
        while (1)
        {
            delay(1000);
            Serial.println("Serious WLED Error! N is Bigger than Max");
        }
        
    }
}


bool WLEDManager::setByIndex(int idx)
{
    int wled_n = idx % numControllers;
    int preset_n = idx / numControllers;
    Serial.printf("wled_n:%i,preset_n:%i\n",wled_n,preset_n);
    if (idx >= STAR_INDEX)
    {
        return wled[wled_n].setState(false);
    }
    

    return wled[wled_n].setPreset(preset_n+1);
}
#endif