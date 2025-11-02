#include "message.h"


wl_status_t connectToWifi(const char * ssid, const char *password, const char *ap_ssid, const char *ap_pass){
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();

  // Try to connect for 10 seconds
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
  } else {
    Serial.println("\nFailed to connect. Starting Access Point...");
    WiFi.softAP(ap_ssid, ap_pass);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());  

  }

  Udp.begin(LOCAL_PORT);
  Serial.printf("Listening for OSC on port %d\n", LOCAL_PORT);
  return WiFi.status();
};




void sendMsg(OSCMessage &msg) {
  Udp.beginPacket(REMOTE_IP, REMOTE_PORT);
  msg.send(Udp);
  Udp.endPacket();
  Serial.print("Sent: ");
  Serial.println(msg.getAddress());
}

void sendHeartbeat(){
   OSCMessage msg("/fon/ping");
   sendMsg(msg);
}


void handleTest(OSCMessage &msg) {
  Serial.print("Received /test ");
  if (msg.size() > 0) Serial.println(msg.getFloat(0));
}

void handleRing(OSCMessage &message){
   int numArgs = message.size();
    if (numArgs == 0) {
        Serial.println("No arguments.");
        return;
    }

    Serial.print("Arguments: ");
    for (int i = 0; i < numArgs; i++) {
        if (message.isInt(i)) {
            Serial.print(message.getInt(i));
        } 
        else if (message.isFloat(i)) {
            Serial.print(message.getFloat(i));
        } 
        // else if (message.isString(i)) {
        //     Serial.print(message.getString(i));
        // } 
        else {
            Serial.print("?");
        }

        if (i < numArgs - 1) Serial.print(", ");
    }
    Serial.println();
  // data.is_ringing = message.getInt(0);
}
