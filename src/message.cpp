#include "message.h"

StateTransitionCallback OSCHandler::stateCallback = nullptr;
OSCHandler* OSCHandler::instance = nullptr;

OSCHandler::OSCHandler():
localPort(LOCAL_PORT),
remoteIp(REMOTE_IP),
remotePort(REMOTE_PORT)
{
  instance = this;
}
void OSCHandler::begin(){
  udp.begin(LOCAL_PORT);
}


void OSCHandler::attachStateTransitionCallback(StateTransitionCallback cb){
  stateCallback = cb;
}

void OSCHandler::poll(){
  int size = udp.parsePacket();
  if(size){
    OSCMessage msgIN;
    while (size--) msgIN.fill(udp.read());

    if(!msgIN.hasError()){
      msgIN.route("/fon/state", handleStateCmd);
    }
  }
}
void OSCHandler::debug(OSCMessage &msg){
int numArgs = msg.size();
    Serial.print("Message");
    Serial.print(msg.getAddress());
    Serial.print("has ");
    Serial.print(numArgs);
    Serial.println(" arguments:");
    for (int i = 0, l = msg.size(); i < l; i++)
    {
          if (msg.isInt(i)) {
            int val = msg.getInt(i);
            Serial.print("Arg "); Serial.print(i); Serial.print(" (int): "); Serial.println(val);
        } else if (msg.isFloat(i)) {
            float val = msg.getFloat(i);
            Serial.print("Arg "); Serial.print(i); Serial.print(" (float): "); Serial.println(val);
        } else if (msg.isString(i)) {
            char buffer[64];
            msg.getString(i, buffer, sizeof(buffer));
            Serial.print("Arg "); Serial.print(i); Serial.print(" (string): "); Serial.println(buffer);
        } else {
            Serial.print("Arg "); Serial.print(i); Serial.println(" (Unknown type)");
        }
    }
    
}

void OSCHandler::composeMsg(OSCMessage &msg, const char *addr)
{
  char buffer[64];
  snprintf(buffer,sizeof(buffer), "/fon%s", addr);
  msg.setAddress(buffer);
}

void OSCHandler::transmitMsg(OSCMessage &msg)
{
  udp.beginPacket(remoteIp,remotePort);
  msg.send(udp);
  udp.endPacket();
  msg.empty();
}


void OSCHandler::send(const char *address, int val)
{
  OSCMessage msg;
  composeMsg(msg,address);
  msg.add(val);
  transmitMsg(msg);
}
void OSCHandler::send(const char *address, float val)
{
  OSCMessage msg;
  composeMsg(msg,address);
  msg.add(val);
  transmitMsg(msg);
}
void OSCHandler::send(const char *address, const char *val)
{
  OSCMessage msg;
  composeMsg(msg,address);
  msg.add(val);
  transmitMsg(msg);
}

void OSCHandler::handleStateCmd(OSCMessage &msg, int addrOffset)
{
  if (msg.isInt(0))
  {
    enum States state = (enum States)msg.getInt(0);
    if(stateCallback) stateCallback(state);
  }else if (msg.size() && !msg.isInt(0))
  {
    instance->send("/error", "expected Int");
  }

  instance->send("/state", getCurrentState());
}
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

  // Udp.begin(LOCAL_PORT);
  // Serial.printf("Listening for OSC on port %d\n", LOCAL_PORT);
  return WiFi.status();
};




void sendMsg(OSCMessage &msg) {
  // Udp.beginPacket(REMOTE_IP, REMOTE_PORT);
  // msg.send(Udp);
  // Udp.endPacket();
  // Serial.print("Sent: ");
  // Serial.println(msg.getAddress());
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
