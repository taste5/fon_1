#include "wled_controller.h"
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
    
    return (httpCode == 200);
}
#endif