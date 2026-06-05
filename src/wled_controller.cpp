#include "wled_controller.h"
#include "custom_keypad.h"
#ifdef ENABLE_WLED

WLEDController::WLEDController(const char* ip)  {
    strncpy(wledIP, ip, WLED_IP_SIZE - 1);
    wledIP[WLED_IP_SIZE - 1] = '\0';
}



void WLEDController::setPreset(int presetNum) {
    snprintf(payload, sizeof(payload), "{\"on\":true,\"ps\":%d}", presetNum);
}

void WLEDController::setState(bool on) {
    snprintf(payload, sizeof(payload), "{\"on\":%s}", on ? "true" : "false");
}

void WLEDController::httpTask() {
    // These are class members, not allocated in task
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST((uint8_t*)payload, strlen(payload));
     if (httpCode > 0) {
        Serial.printf("[WLED] Response code: %d\n", httpCode);
    } else {
        Serial.printf("[WLED] Error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
    
}

void WLEDController::sendCommand() {
   snprintf(url, WLED_URL_SIZE, "http://%s/json/state", wledIP);
    
    xTaskCreatePinnedToCore(
        httpTaskWrapper,
        "wledTask",
        4096,
        (void*)this,
        1,
        NULL,
        1
    );

    #ifdef WLED_DEBUG
    Serial.println("=== sendCommand Debug ===");
    Serial.printf("Target IP: %s\n", wledIP);
    Serial.printf("Payload: %s\n", payload);  // or however you build it
    
    
    Serial.println("======================");
    #endif
    
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


void WLEDManager::setByIndex(int idx)
{
    
    int wled_n = idx % numControllers;
    int preset_n = idx / numControllers;
    Serial.printf("wled_n:%i,preset_n:%i\n",wled_n,preset_n);
    if (idx >= STAR_INDEX)
    {
         wled[wled_n].setState(false);
    }
    else{
            wled[wled_n].setPreset(preset_n+1);
    }
    

     wled[wled_n].sendCommand();
}
#endif