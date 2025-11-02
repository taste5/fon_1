#ifndef __MESSAGE_H__
#define __MESSAGE_H__


#include <Arduino.h>
#include <OSCMessage.h>
#include "globals.h"
#include "config.h"
#include <WiFiUdp.h>
#include <WiFi.h>

wl_status_t connectToWifi(const char * ssid, const char *password, const char *ap_ssid, const char *ap_pass);

extern WiFiUDP Udp;
#endif 