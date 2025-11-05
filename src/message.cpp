#include "message.h"

StateTransitionCallback OSCHandler::stateCallback = nullptr;
OSCHandler* OSCHandler::instance = nullptr;

OSCHandler::OSCHandler(unsigned int localPort, unsigned int remotePort, const char *remoteIp):
localPort(localPort),
remotePort(remotePort),
connectionState(NOT_CONNECTED)
{
  instance = this;
  this->remoteIp = makeIP(remoteIp);
}
void OSCHandler::begin(){
  this->connectToWifi();
  udp.begin(LOCAL_PORT);
  Serial.printf("Listening for OSC on port %d\n", LOCAL_PORT);
}


IPAddress OSCHandler::makeIP(const char *ipstr){
  IPAddress ip;
  ip.fromString(ipstr);
  return ip;
}

void OSCHandler::attachStateTransitionCallback(StateTransitionCallback cb){
  stateCallback = cb;
}

void OSCHandler::ping(){
  static unsigned long pingTimer = 0;
  const int pingIntervalMS = 0xFFFF;
  if(millis() - pingTimer > pingIntervalMS)
  {
    this->send("/ping", 0);
    pingTimer = millis();
  }
}

void OSCHandler::poll(){
  if (connectionState == NOT_CONNECTED)
  {
    return;
  }
  
  int size = udp.parsePacket();
  if(size){
    OSCMessage msgIN;
    while (size--) msgIN.fill(udp.read());

    if(!msgIN.hasError()){
      this->debug(msgIN);
      msgIN.route("/fon/state", handleStateCmd);
      msgIN.route("/fon/ring", handleStateCmd);
      msgIN.route("/fon/pickup", handleStateCmd);
      msgIN.route("/fon/idle", handleStateCmd);
    }
  }
}
void OSCHandler::debug(OSCMessage &msg){
int numArgs = msg.size();
    Serial.print(msg.getAddress());
    Serial.print(" ");
    for (int i = 0, l = msg.size(); i < l; i++)
    {
          if (msg.isInt(i)) {
            int val = msg.getInt(i);
            Serial.print(" (int): "); Serial.print(val);
        } else if (msg.isFloat(i)) {
            float val = msg.getFloat(i);
            Serial.print(" (float): "); Serial.print(val);
        } else if (msg.isString(i)) {
            char buffer[64];
            msg.getString(i, buffer, sizeof(buffer));
            Serial.print(" (string): "); Serial.print(buffer);
        } else {
            Serial.print(i); Serial.print(" (Unknown type)");
        }
    }
    Serial.println();
    
}


void OSCHandler::transmitMsg(OSCMessage &msg)
{
  if (connectionState == NOT_CONNECTED)
  {
    Serial.println("Warning. Sending Aborted because there is no Connection");
    return;
  }
  
  Serial.print("Sent message: ");
  this->debug(msg);
  udp.beginPacket(remoteIp,remotePort);
  msg.send(udp);
  udp.endPacket();
  msg.empty();
}


void OSCHandler::send(const char *address, int val)
{
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "/fon%s%s", (address[0] == '/') ? "" : "/", address);
  OSCMessage msg(buffer);
  msg.add(val);
  transmitMsg(msg);
}
void OSCHandler::send(const char *address, float val)
{
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "/fon%s%s", (address[0] == '/') ? "" : "/", address);
  OSCMessage msg(buffer);
  msg.add(val);
  transmitMsg(msg);
}
void OSCHandler::send(const char *address, const char *val)
{
 char buffer[64];
  snprintf(buffer, sizeof(buffer), "/fon%s%s", (address[0] == '/') ? "" : "/", address);
  OSCMessage msg(buffer);
  msg.add(val);
  transmitMsg(msg);
}


void OSCHandler::connectToWifi(){
  Serial.println("Connecting to WiFi...");
  WiFi.begin(credentials.ssid,credentials.password);
  
  unsigned long startAttemptTime = millis();
  
  // Try to connect for 10 seconds
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    Serial.print(".");
    delay(500);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    connectionState = CONNECTED;
  } else {
    Serial.println("\nFailed to connect. Starting Access Point...");
    WiFi.softAP(credentials.ap_ssid, credentials.ap_pass);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
    connectionState = AP_OPEN;
  }
  
  
  
  
};
void OSCHandler::handleStateCmd(OSCMessage &msg, int addrOffset)
{
 const char *subAddr = msg.getAddress() + 5; // 5 = /fon/
 char c = subAddr[0];
 Serial.print(subAddr);

 switch (c)
 {
 case 'r': //ring
    if(stateCallback) stateCallback(STATE_RINGING);
  break;
  case 'p': //pickup
    if(stateCallback) stateCallback(STATE_PICKEDUP);
  break;
  case 'i': //idle
    if(stateCallback) stateCallback(STATE_IDLE);
  break; 
  default:
    if (msg.isInt(0))
    {
      enum States state = (enum States)msg.getInt(0);
      if(stateCallback) stateCallback(state);
    }else if (msg.size() && !msg.isInt(0))
    {
      instance->send("/error", "expected Int");
    }
  break;
  }

  instance->send("/state", getCurrentState());
}
