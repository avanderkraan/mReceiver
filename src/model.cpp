#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal

//#include <ESP8266mDNS.h>
#include "handlemDNS.h"
#include <WiFiUdp.h>

#include "updateOverHTTP.h"

#include "settings.h"
#include "handleWebServer.h"
#include "handleHTTPClient.h"
#include "WiFiSettings.h"
#include <AccelStepper.h>

const int motorPin1 = D0;   // IN1
const int motorPin2 = D5;   // IN2
const int motorPin3 = D6;   // IN3
const int motorPin4 = D7;   // IN4

// WIFI URL: http://192.168.4.1/ or http://model.local/
/////////////////////
// Pin Definitions //
/////////////////////
// On a ESP8266-12 GPIO0 is used, physical name is pin D0
// On a ESP8266-12 GPIO5 is used, physical name is pin D5

// when using D0 only one direction of receiver works, so now using D8

// D5 gives troubles when it is high at the start.

const uint8_t BUTTON = D1;           // was D8 // Digital pin to read button-push

const uint8_t ACCESSPOINT_LED = D3;  // was D1 
const uint8_t STATION_LED = D2;      // was D2

// variables for reset to STA mode
const uint16_t NO_STA_COUNTER_MAX = 6000; // with a delay of 50 ms the max pause time is 5 minutes
uint16_t no_sta_counter = 0;
bool eepromStartModeAP = false;     // see setup, holds the startmode from eeprom

const uint32_t RELAX_PERIOD = 2;    // Is also a small energy saving, in milliseconds
const uint32_t TOO_LONG = 60000;    // after this period the pulsesPerMinute = 0 (in milliseconds)
bool permissionToDetect = false;    // all sensors must have had a positive value 

uint32_t startPulse = millis();     // set the offset time for a passing a pulse
uint32_t pulsesPerMinute = 0;       // holds the value of pulses per minute
uint32_t revolutions = 0;           // holds the value of revolutions of the first axis, calculated with ratio
uint32_t viewPulsesPerMinute = 0;   // holds the value of ends per minute calculated with ratio

String INDEPENDENT = "independent"; // means that motor is driven independent of any response

Settings settings = Settings();
Settings* pSettings = &settings;

// stepsPerrevolution, maxSpeed, direction, motorInterfaceType are coming from settings/database
// stepsPerRevolution for your motor 28BYJ-48 is 2038 for FULLxWIRE
int16_t stepsPerRevolution = pSettings->getStepsPerRevolution();  // 4096 change this to fit the number of steps per revolution
int16_t maxSpeed = pSettings->getMaxSpeed();  // 1
int8_t direction = pSettings->getDirection();    // 1 or -1, some motors are wired reversed
uint8_t motorInterfaceType = pSettings->getMotorInterfaceType(); //AccelStepper::HALF4WIRE;
int16_t motorSpeedStepper = 0;
int16_t previousMotorSpeedStepper = motorSpeedStepper;

AccelStepper myStepper = AccelStepper(motorInterfaceType, motorPin1, motorPin3, motorPin2, motorPin4);

//////////////////////
// WiFi Definitions //
//////////////////////
WiFiSettings wifiSettings = WiFiSettings(pSettings);
WiFiSettings* pWifiSettings = &wifiSettings;

//////////////////////
// AsyncHTTPrequest //
//////////////////////
asyncHTTPrequest aRequest;
long lastSendMillis;

// detectButtonFlag lets the program know that a network-toggle is going on
bool detectButtonFlag = false;

// detectUpdateFlag is True is an update from the server is requested
bool detectUpdateFlag = false;

// updateSucceeded is true is the update succeeded or if a restart is asked, so a restart can be done
bool updateSucceeded = false;

// detectInfoRequest is True if info is requested by the server
bool detectInfoRequest = false;

// Forward declaration
void setupWiFi();
void showSettings();
void switchToAccessPoint();
void handleShowWiFiMode();
void initServer();

void ICACHE_RAM_ATTR detectButton();
void buttonInterruptOn();
void buttonInterruptOff();

void toggleWiFi();
/*
2^8 = 256
2^16 = 65536
2^32 = 4294967296
2^64 = 18446744073709551616
*/

ESP8266WebServer server(80);
WiFiClient wifiClient;
//MDNSResponder mdns;

// start Settings and EEPROM stuff
void saveSettings() {
  pSettings->saveSettings();
  showSettings();
}

void getSettings() {
  pSettings->getSettings();
  showSettings();
}

void eraseSettings() {
  pSettings->eraseSettings();
  pSettings->getSettings();   // otherwise the previous values of Settings are used
  showSettings();
}

void initSettings() {
  pSettings->initSettings();
  pSettings->getSettings();   // otherwise the previous values of Settings are used
  showSettings();
}
// end Settings and EEPROM stuff

void setupWiFi(){
  digitalWrite(STATION_LED, HIGH);
  digitalWrite(ACCESSPOINT_LED, HIGH);

  WiFi.mode(WIFI_AP);
  String myssid = pWifiSettings->readAccessPointSSID();
  String mypass = pWifiSettings->readAccessPointPassword();

  if (myssid == "")
  {
    myssid = "ESP-" + WiFi.macAddress();
    pWifiSettings->setAccessPointSSID(myssid);
  }

  IPAddress local_IP(192,168,4,1);
  IPAddress gateway(192,168,4,1);
  IPAddress subnet(255,255,255,0);

  Serial.print("Setting soft-AP ... ");
  // mypass needs minimum of 8 characters, otherwise it shall fail !
  Serial.println(WiFi.softAP(myssid,mypass,3,0) ? "Ready" : "Failed!");
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
  Serial.print("Connecting to AP mode");

  delay(500);
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  Serial.println(WiFi.softAPmacAddress());

  digitalWrite(ACCESSPOINT_LED, HIGH);
  digitalWrite(STATION_LED, LOW);

  pSettings->beginAsAccessPoint(true);
}

void setupWiFiManager () {
  bool networkConnected = false;

  digitalWrite(STATION_LED, HIGH);
  digitalWrite(ACCESSPOINT_LED, HIGH);

  String mynetworkssid = pWifiSettings->readNetworkSSID();
  if (mynetworkssid != "") {
    String mynetworkpass = pWifiSettings->readNetworkPassword();
    WiFi.mode(WIFI_STA);
    WiFi.begin(mynetworkssid, mynetworkpass); 

    Serial.print("Connecting to a WiFi Network");
    int toomuch = 30;  //gives 30 seconds to connect to a Wifi network
    while ((WiFi.status() != WL_CONNECTED) && (toomuch > 0))
    {
      delay(1000);
      Serial.print(".");
      toomuch -=1;
    }
    if (toomuch > 0) {
      Serial.println();

      Serial.print("Connected, IP address: ");
      Serial.println("local ip address");
      Serial.println(WiFi.localIP());
      Serial.println(WiFi.gatewayIP());
      Serial.println(WiFi.macAddress());
    
      networkConnected = true;
      pSettings->setLastNetworkIP(WiFi.localIP().toString());

      digitalWrite(ACCESSPOINT_LED, LOW);
      digitalWrite(STATION_LED, HIGH);
      pSettings->beginAsAccessPoint(false);
    }
  }
  if (networkConnected == false) {
    // no network found, fall back to access point
    Serial.println("no network SSID found/selected, fall back to AccessPoint mode");
    switchToAccessPoint();
  }
}

void resetWiFiManagerToFactoryDefaults () {

  // WiFi.disconnect(true);  // true argument should also erase ssid and password
  // https://www.pieterverhees.nl/sparklesagarbage/esp8266/130-difference-between-esp-reset-and-esp-restart
  Serial.println("try to disconnect works only when WiFi.begin() was successfully called");
  int toomuch = 2;
  while (toomuch > 0) {
    int getValue = WiFi.disconnect(true);
    if (getValue != 0) {
      toomuch = 0;
    }
    Serial.println(String(getValue));
    delay(3000);
    Serial.println("waited 3 seconds");
    toomuch -= 1;
  }
}

void switchToAccessPoint() {
  pSettings->beginAsAccessPoint(!  pSettings->beginAsAccessPoint());  // toggle
  handleShowWiFiMode();
  delay(pSettings->WAIT_PERIOD);

  server.close();
  resetWiFiManagerToFactoryDefaults();
  delay(pSettings->WAIT_PERIOD);

  setupWiFi();
  delay(pSettings->WAIT_PERIOD);

  initServer();

  mDNSnotifyAPChange();
  // end domain name server check
}

void switchToNetwork() {
  handleShowWiFiMode();
  delay(pSettings->WAIT_PERIOD);

  server.close();
  resetWiFiManagerToFactoryDefaults();
  delay(pSettings->WAIT_PERIOD);

  setupWiFiManager();
  delay(pSettings->WAIT_PERIOD);

  delay(pSettings->WAIT_PERIOD);
  initServer();

  mDNSnotifyAPChange();

}

void delayInMillis(uint8_t ms)
{
  for (uint8_t i = 0; i <= ms; i++)
  {
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
  }
}

void ICACHE_RAM_ATTR detectButton() {  // ICACHE_RAM_ATTR is voor interrupts
  // this function is called after a change of pressed button  
  buttonInterruptOff();  // to prevent exception

  delayInMillis(10);      // prevent bounce
  
  if (digitalRead(BUTTON) == LOW)
  {
    detectButtonFlag = true;
    // only toggle between AP and STA by using the button, not saving in EEPROM
  }
  buttonInterruptOn();  // to prevent exception
}

void buttonInterruptOn() {
  attachInterrupt(digitalPinToInterrupt(BUTTON), detectButton, FALLING);
}

void buttonInterruptOff() {
  detachInterrupt(BUTTON);
}

void handleShowWiFiMode()
{
  if (pSettings->getLanguage() == "NL")
  {
    showWiFiMode_nl(server, pSettings);
  }
  else
  {
    showWiFiMode(server, pSettings);
  }
}

void handleWiFi() {
  if (pSettings->getLanguage() == "NL")
  {
    wifi_nl(server, pSettings, pWifiSettings);
  }
  else
  {
    wifi(server, pSettings, pWifiSettings);
  }
}

void handleSpin() {
  if (pSettings->getLanguage() == "NL")
  {
    spin_nl(server, pSettings);
  }
  else
  {
    spin(server, pSettings);
  }
}

/*
void handleDevice() {
  if (pSettings->getLanguage() == "NL")
  {
    device_nl(server, pSettings);
  }
  else
  {
    device(server, pSettings);
  }
}
*/

void handleSse() {
  sse(server, pSettings, revolutions, viewPulsesPerMinute);
}

void mydebug() {
  String result = "";
  String myIP = "";
  result += "IP address: ";
  if (WiFi.getMode() == WIFI_AP)
  {
    myIP = WiFi.softAPIP().toString();
  }
  if (WiFi.getMode() == WIFI_STA)
  {
    myIP = WiFi.localIP().toString();
  }

  result += myIP;
  result += "\r\n";

  Serial.println("wifi gegevens");
  Serial.print("readAccessPointSSID: ");
  Serial.println(pWifiSettings->readAccessPointSSID());
  Serial.print("readNetworkSSID: ");
  Serial.println(pWifiSettings->readNetworkSSID());

  Serial.print("Chip ID: ");
  Serial.println(ESP.getFlashChipId());
 
  Serial.print("Chip Real Size: ");
  Serial.println(ESP.getFlashChipRealSize());
 
  Serial.print("Chip Size: ");
  Serial.println(ESP.getFlashChipSize());
 
  Serial.print("Chip Speed: ");
  Serial.println(ESP.getFlashChipSpeed());
 
  Serial.print("Chip Mode: ");
  Serial.println(ESP.getFlashChipMode());

  Serial.print("firmware version: ");
  Serial.println(pSettings->getFirmwareVersion());

  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

  Serial.print("target server: ");
  Serial.println(pSettings->getTargetServer());

  Serial.print("target port: ");
  Serial.println(pSettings->getTargetPort());

  Serial.print("target path: ");
  Serial.println(pSettings->getFactoryTargetPath());

  Serial.print("connected roleModel: ");
  Serial.println(pSettings->getRoleModel());

  Serial.print("motor steps per revolution: ");
  Serial.println(pSettings->getStepsPerRevolution());
  
  Serial.print("motor maximum speed: ");
  Serial.println(pSettings->getMaxSpeed());
  
  Serial.print("motor direction: ");
  Serial.println(pSettings->getDirection());
  
  Serial.print("motor motorInterfaceType: ");
  Serial.println(pSettings->getMotorInterfaceType());

  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/html", result);
}

String updateFirmware(String requestedVersion)
{
  buttonInterruptOff();

  digitalWrite(STATION_LED, HIGH);
  digitalWrite(ACCESSPOINT_LED, HIGH);

  String serverUrl = pSettings->getTargetServer();
  uint16_t serverPort = pSettings->getTargetPort();
  String uploadScript = "/update/updateFirmware/?device=model&version=" + requestedVersion;
  String version = pSettings->getFirmwareVersion();
  String result = updateOverHTTP(wifiClient, serverUrl, serverPort, uploadScript, version);

  if (result ==UPDATEOVERHTTP_OK)
  {
    updateSucceeded = true;
  }
  else
  {
    // restore settings
    buttonInterruptOn();
    if (WiFi.getMode() == WIFI_STA)
    {
      digitalWrite(STATION_LED, HIGH);
      digitalWrite(ACCESSPOINT_LED, LOW);
    }
    else
    {
      digitalWrite(STATION_LED, LOW);
      digitalWrite(ACCESSPOINT_LED, HIGH);
    }
  return result;
  }
  
  digitalWrite(STATION_LED, LOW);
  digitalWrite(ACCESSPOINT_LED, LOW);
  return result;
}

void handleVersion() {
  uint8_t argumentCounter = 0;
  String result = "";
  String result_nl = "";

  if (server.method() == HTTP_POST)
  {
    argumentCounter = server.args();
    String name = "";
    for (uint8_t i=0; i< server.args(); i++){
      if (server.argName(i) == "name") {
        name = server.arg(i);
      }
    }
    // search name 
    if (name == "update")
    {
      if (argumentCounter > 0)
      {
        result = updateFirmware("latest");
        if (result == UPDATEOVERHTTP_OK)
        {
          updateSucceeded = true;
        }
      }
    }
  }
  if (pSettings->getLanguage() == "NL")
  {
    if (result == UPDATEOVERHTTP_FAILED)
    {
      result_nl = "[update] Update mislukt";
    }
    if (result == UPDATEOVERHTTP_NO_UPDATE)
    {
      result_nl = "[update] Geen update aanwezig";
    }
    if (result == UPDATEOVERHTTP_NO_INTERNET)
    {
      result_nl = "[update] Geen connectie met de server aanwezig";
    }
    if (result == UPDATEOVERHTTP_OK)
    {
      result_nl = "[update] Update ok";
    }
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Connection", "keep-alive");
    server.sendHeader("Pragma", "no-cache");
    server.send(200, "text/html", result);
  }
  else
  {
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Connection", "keep-alive");
    server.sendHeader("Pragma", "no-cache");
    server.send(200, "text/html", result);
  }
}

void handleRestart() {
  uint8_t argumentCounter = 0;
  String result = "";
  String result_nl = "";

  if (server.method() == HTTP_POST)
  {
    argumentCounter = server.args();
    String name = "";
    for (uint8_t i=0; i< server.args(); i++){
      if (server.argName(i) == "name") {
        name = server.arg(i);
      }
    }
    // search name 
    if (name == "restart")
    {
      if (argumentCounter > 0)
      {
        updateSucceeded = true;
        result = "Restart completed";
        result_nl = "Restart compleet";
      }
    }
  }
  if (pSettings->getLanguage() == "NL")
  {
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Connection", "keep-alive");
    server.sendHeader("Pragma", "no-cache");
    server.send(200, "text/html", result_nl);
  }
  else
  {
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Connection", "keep-alive");
    server.sendHeader("Pragma", "no-cache");
    server.send(200, "text/html", result);
  }
}
/* void alive must be used in clients only
but for now, in develop-phase it is allowed here
TODO remove this when clients are available to test
*/
void getMDNS() {
  String firstFreeHostname = findFirstFreeHostname();

  /* used to answer a xhr call from the browser that is connected to the server */
  String result = "";

  result += firstFreeHostname;
  result += "<";
  result += pSettings->getRoleModel();
  result += ">";
  result += "\r\n";

  String allowServer = pSettings->getTargetServer() + ":" + pSettings->getTargetPort();

  if ((pSettings->getTargetPort() == 80) || (pSettings->getTargetPort() == 443))
  {
    allowServer = pSettings->getTargetServer();
  }
      
  uint8_t argumentCounter = 0;
  argumentCounter = server.args();
  if (argumentCounter == 1) {
    for (uint8_t i=0; i< server.args(); i++){
      if (server.argName(i) == "name") {
        allowServer = server.arg(i);
        int8_t index = allowServer.lastIndexOf("/");
        if (index > -1) {
          if ((uint8_t)index == allowServer.length() -1) {
            allowServer = allowServer.substring(0, index);
          }
        }
      }
    }
  }

  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Access-Control-Allow-Origin", allowServer);
  server.send(200, "text/html", result);
}

void getMyIP() {

  /* used to answer a xhr call from the browser that is connected to the server */
  String result = "model_";
 
  String myIP = "";

  if (WiFi.getMode() == WIFI_AP)
  {
    myIP = WiFi.softAPIP().toString();
  }
  if (WiFi.getMode() == WIFI_STA)
  {
    myIP = WiFi.localIP().toString();
  }
  result += myIP;
  result += "<";
  result += pSettings->getRoleModel();
  result += ">";
  result += "\r\n";

  String allowServer = pSettings->getTargetServer() + ":" + pSettings->getTargetPort();

  if ((pSettings->getTargetPort() == 80) || (pSettings->getTargetPort() == 443))
  {
    allowServer = pSettings->getTargetServer();
  }

  uint8_t argumentCounter = 0;
  argumentCounter = server.args();
  if (argumentCounter == 1) {
    for (uint8_t i=0; i< server.args(); i++){
      if (server.argName(i) == "name") {
        allowServer = server.arg(i);
        int8_t index = allowServer.lastIndexOf("/");
        if (index > -1) {
          if ((uint8_t)index == allowServer.length() -1) {
            allowServer = allowServer.substring(0, index);
          }
        }
      }
    }
  }

  Serial.println(allowServer);

  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Access-Control-Allow-Origin", allowServer);
  server.send(200, "text/html", result);
}

void handleRoleModel() {
  //uint8_t argumentCounter = 0;
  String result = "";
  String result_nl = "";

  if (server.method() == HTTP_GET)
  {
    //argumentCounter = server.args();  // if argumentCounter > 0 then save
    String roleModel = "";
    String _speed = "";
    for (uint8_t i=0; i< server.args(); i++){
      if (server.argName(i) == "id") {
        if (server.arg(i).length() > 32) {
          roleModel = server.arg(i).substring(0, 32);
        }
        else{
          roleModel = server.arg(i);
        }   
      }
      if (roleModel == INDEPENDENT)
      {
        if (server.argName(i) == "speed") {
          if (server.arg(i).length() < 6) { 
            _speed = server.arg(i);     // speed in revolutions per hour
            motorSpeedStepper = round(_speed.toInt() * stepsPerRevolution / 3600);
          }
        }
      }
    }
    if (pSettings->getRoleModel() != roleModel)
    {
      pSettings->setRoleModel(roleModel);
      result += "rolemodel is saved";
      result_nl += "rolmodel is opgeslagen";
    }
  }
  if (pSettings->getLanguage() == "NL")
  {
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Connection", "keep-alive");
    server.sendHeader("Pragma", "no-cache");
    server.send(200, "text/plain", result_nl);
  }
  else
  {
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Connection", "keep-alive");
    server.sendHeader("Pragma", "no-cache");
    server.send(200, "text/plain", result);
  }
}

void showSettings() {
  if (pSettings->getLanguage() == "NL")
  {
    showSavedSettings_nl(server, pSettings);
  }
  else
  {
    showSavedSettings(server, pSettings);
  }
}

void handleHelp() {
  if (pSettings->getLanguage() == "NL")
  {
    help_nl(server, pSettings);
  }
  else
  {
    help(server, pSettings);
  }
}

void handleLanguage() {
  uint8_t argumentCounter = 0;
  String result = "";
  String result_nl = "";

  if (server.method() == HTTP_POST)
  {
    argumentCounter = server.args();  // if argumentCounter > 0 then save
    String name = "";
    String language = "";
    for (uint8_t i=0; i< server.args(); i++){
      if (server.argName(i) == "name") {
        name = server.arg(i);
      }
      if (server.argName(i) == "language") {
        language = server.arg(i);
      }
    }
    // search name 
    if (name == "help")
    {
      if (argumentCounter > 0)
      {
        pSettings->setLanguage(language);
      }
    }
  }
  if (pSettings->getLanguage() == "NL")
  {
    server.send(200, "text/plain", result_nl);
  }
  else
  {
    server.send(200, "text/plain", result);
  }
}

void handleNetworkSSID() {
  // creates a list of {ssid, including input field , dBm}
  String result = "";
  int numberOfNetworks = WiFi.scanNetworks();
  for(int i =0; i<numberOfNetworks; i++){ 

    if (i > 0) {
      result += ",";
    }
    result += "{ssid:";
    result += "'<span><input type=\"radio\" name=\"networkSSID\" onclick=\"selectNetworkSSID(this)\" value=\"";
    result += WiFi.SSID(i);
    result += "\">";
    result += WiFi.SSID(i);
    result += "</span>";
    result += "',";
    result += "dBm:'";
    result += WiFi.RSSI(i);
    result += "'}";
  }
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/html", result);
}

void handleWifiConnect() {
  uint8_t argumentCounter = 0;
  String result = "";
  String result_nl = "";

  if (server.method() == HTTP_POST)
  {
    argumentCounter = server.args();  // if argumentCounter > 0 then save
    String name = "";
    String ssid = "";
    String password = "";
    String target = "";               // for action Erase
    for (uint8_t i=0; i< server.args(); i++){
      if (server.argName(i) == "name") {
        name = server.arg(i);
      }
      if (server.argName(i) == "ssid") {
        ssid = server.arg(i);
      }
      if (server.argName(i) == "password") {
        password = server.arg(i);
      }
      if (server.argName(i) == "target") {
        target = server.arg(i);
      }
    }
    // zoek name (is ap of network en dan de ssid en password)
    if (name == "ap")
    {
      pWifiSettings->setAccessPointSSID(ssid);
      pWifiSettings->setAccessPointPassword(password);
      if (argumentCounter > 0) {
        pWifiSettings->saveAuthorizationAccessPoint();
        result += "Access Point data has been saved\n";
        result_nl += "Access Point gegevens zijn opgeslagen\n";
      }
    }
    if (name == "network")
    {
      pWifiSettings->setNetworkSSID(ssid);
      pWifiSettings->setNetworkPassword(password);
      if (argumentCounter > 0) {
        pWifiSettings->saveAuthorizationNetwork();
        result += "Network connection data has been saved\n";
        result_nl += "Netwerk verbindingsgegevens zijn opgeslagen\n";
      }
    }
    if (name == "erase")
    {
      if (argumentCounter > 0) {
        if (target == "eraseAPData")
        {
          pWifiSettings->eraseAccessPointSettings();
          result += "Access Point data has been erased\n";
          result_nl += "Access Point gegevens zijn gewist\n";
        }
        if (target == "eraseNetworkData")
        {
          pWifiSettings->eraseNetworkSettings();
          result += "Network connection data has been erased\n";
          result_nl += "Netwerk verbindingsgegevens zijn gewist\n";
        }
        if (target == "eraseWiFiData")
        {
          pWifiSettings->eraseWiFiSettings();
          result += "Access Point data and Network connection data has been erased\n";
          result_nl += "Access Point gegevens en Netwerk verbindingsgegevens zijn gewist\n";
        }
      }
    }
  }
  if (pSettings->getLanguage() == "NL")
  {
    server.send(200, "text/plain", result_nl);
  }
  else
  {
    server.send(200, "text/plain", result);
  }
  Serial.println(result);
}

void handleDeviceSettings()
{
  uint8_t argumentCounter = 0;
  String result = "";
  String result_nl = "";

  if ((server.method() == HTTP_POST) || (server.method() == HTTP_GET))
  {
    // extract the settings-data and take action
    argumentCounter = server.args();  // if argumentCounter > 0 then saveConfigurationSettings
    String _name = "";
    String _startWiFiMode = String(pSettings->beginAsAccessPoint());
    String _targetServer = pSettings->getTargetServer();
    String _targetPort = String(pSettings->getTargetPort());
    String _targetPath = pSettings->getTargetPath();
    for (uint8_t i=0; i< server.args(); i++){
      if (server.argName(i) == "name") {
        _name = server.arg(i);
      }
      if (server.argName(i) == "startWiFiMode") {
        _startWiFiMode = server.arg(i);
      }
      if (server.argName(i) == "targetServer") {
        _targetServer = server.arg(i);
      }
      if (server.argName(i) == "targetPort") {
        _targetPort = server.arg(i);
      }
      if (server.argName(i) == "targetPath") {
        _targetPath = server.arg(i);
      }
    }
    // zoek name (is device, targetServer of targetserverData en dan de andere parameters)
    if (_name == "device")
    {
      if (_startWiFiMode == "ap") {
        pSettings->beginAsAccessPoint(true);
      }
      if (_startWiFiMode == "network") {
        pSettings->beginAsAccessPoint(false);
      }
    }
    if (_name == "targetServer")
    {
      pSettings->setTargetServer(_targetServer);
      pSettings->setTargetPort(_targetPort);
      pSettings->setTargetPath(_targetPath);
    }
    if (argumentCounter > 0) {
      pSettings->saveConfigurationSettings();
      result += "Device data has been saved\n";
      result_nl += "Apparaatgegevens zijn opgeslagen\n";
    }
  }
  if (pSettings->getLanguage() == "NL")
  {
    server.send(200, "text/plain", result_nl);
  }
  else
  {
    server.send(200, "text/plain", result);
  }
  Serial.println(result);
}

void handleSpinSettings()
{
  uint8_t argumentCounter = 0;
  String result = "";
  String result_nl = "";

  if (server.method() == HTTP_POST)
  {
    // extract the settings-data and take action
    argumentCounter = server.args();  // if argumentCounter > 0 then saveConfigurationSettings
    String _name = "";
    String _spinMode = "";
    String _roleModelSpeed = "";
    String _roleModelCode = "";
    for (uint8_t i=0; i< server.args(); i++){
      if (server.argName(i) == "name") {
        _name = server.arg(i);
      }
      if (server.argName(i) == "spinMode") {
        _spinMode = server.arg(i);
      }
      if (server.argName(i) == "roleModelSpeed") {
        _roleModelSpeed = server.arg(i);
      }
      if (server.argName(i) == "roleModelCode") {
        _roleModelCode = server.arg(i);
      }
    }
    // zoek name (is device, targetServer of targetserverData en dan de andere parameters)
    if (_name == "spin")
    {
      if (_spinMode == INDEPENDENT) {
        pSettings->setRoleModel(INDEPENDENT);
        if (_roleModelSpeed.toInt() * stepsPerRevolution / 60 < maxSpeed) {
          motorSpeedStepper = round(_roleModelSpeed.toInt() * stepsPerRevolution / 60);
         }
         else {
           motorSpeedStepper = maxSpeed;
         }
      }
      if (_spinMode == "connected")
      {
        pSettings->setRoleModel(_roleModelCode); 
        if (WiFi.getMode() == WIFI_AP)
        {
          motorSpeedStepper = 0;
        }
      }
      if (_spinMode == "stop")
      {
        pSettings->setRoleModel("None");
        motorSpeedStepper = 0;
      }
    }
    if (argumentCounter > 0) {
      pSettings->saveConfigurationSettings();
      result += "Device data has been saved\n";
      result_nl += "Apparaatgegevens zijn opgeslagen\n";
    }
  }
  if (pSettings->getLanguage() == "NL")
  {
    server.send(200, "text/plain", result_nl);
  }
  else
  {
    server.send(200, "text/plain", result);
  }
  Serial.println(result);
}

void smoothAcceleration() {
    if (motorSpeedStepper - previousMotorSpeedStepper > 20)
    {
      motorSpeedStepper = previousMotorSpeedStepper + 20;
      previousMotorSpeedStepper = motorSpeedStepper;
    }
    if (previousMotorSpeedStepper - motorSpeedStepper > 20)
      {
      motorSpeedStepper = previousMotorSpeedStepper - 20;
      previousMotorSpeedStepper = motorSpeedStepper;
    }
}

String getValueFromJSON(String key, String responseData)
{
  int16_t keyIndex = responseData.indexOf(key);
  if (keyIndex > -1)
  {
    int16_t start = responseData.indexOf(":", keyIndex);
    int16_t end = responseData.indexOf(",", start);
    if (end == -1) {
      end = responseData.indexOf("}", start);
    }
    String value = responseData.substring(start + 1, end);
    value.trim();
    value.replace("\"", "");
    return value;
  }
  return "";
}

void processServerData(String responseData) {
  /* data should come in JSON format */
  //Serial.println(responseData);
  String proposedUUID = getValueFromJSON("pKey", responseData);
  if ((proposedUUID != "") && (pSettings->getDeviceKey() != proposedUUID))
  {
    pSettings->setDeviceKey(proposedUUID);
    pSettings->saveConfigurationSettings(); // save to EEPROM
  }

  String pushFirmwareVersion = getValueFromJSON("pFv", responseData);
  if (pushFirmwareVersion != "")
  {  
    detectUpdateFlag = true;
  }

  String requestForInfo = getValueFromJSON("i", responseData);
  if (requestForInfo != "")
  {
    detectInfoRequest = true;
  }
  String rph = getValueFromJSON("rph", responseData);
  
  String myStepsPerRevolution = getValueFromJSON("spr", responseData);
  String myMaxSpeed = getValueFromJSON("ms", responseData);
  String myDirection = getValueFromJSON("d", responseData);
  String myMotorInterfaceType = getValueFromJSON("mit", responseData);

  if ((myStepsPerRevolution != "") && (myMaxSpeed != "") && (myDirection != "") && (myMotorInterfaceType != ""))
  {
    if ((myStepsPerRevolution != String(pSettings->getStepsPerRevolution())) &&
        (myMaxSpeed != String(pSettings->getMaxSpeed())) &&
        (myDirection != String(pSettings->getDirection())) &&
        (myMotorInterfaceType != String(pSettings->getMotorInterfaceType())) )
        {
          pSettings->setStepsPerRevolution((uint16_t)myStepsPerRevolution.toInt());
          pSettings->setMaxSpeed((uint16_t)myMaxSpeed.toInt());
          pSettings->setDirection((int8_t)myDirection.toInt());
          pSettings->setMotorInterfaceType((uint8_t)myMotorInterfaceType.toInt());

          pSettings->saveMotorSettings();
          ESP.restart();
        }

    //myStepper = AccelStepper(motorInterfaceType, motorPin1, motorPin3, motorPin2, motorPin4);
  }

  // TODO: could be used on a display:
  //String name = getValueFromJSON("name", responseData);
  //String message = getValueFromJSON("message", responseData);
  // end TODO:

  //if (pSettings->getRoleModel() != INDEPENDENT)
  //{
  //  motorSpeedStepper = 0;
  //}
  if (rph != "")
  {
    uint16_t speedValue = (uint16_t)rph.toInt();
    // setSpeed is per second, rph is per hour, so divide by 3600
    if (speedValue * stepsPerRevolution / 3600 < maxSpeed) {
       motorSpeedStepper = round(speedValue * stepsPerRevolution / 3600);
    }
    else {
      motorSpeedStepper = maxSpeed;
    }
  }
}

void toggleWiFi()
{
  // only toggle by using the button, not saving in EEPROM
  pSettings->beginAsAccessPoint(!  pSettings->beginAsAccessPoint());  // toggle
  if (pSettings->beginAsAccessPoint() == true)
  {
    //switchToAccessPoint();
    setupWiFi();        // local network as access point
  }
  else
  {
    //switchToNetwork();
    setupWiFiManager();             // part of local network as station
  }
}

void initHardware()
{
  Serial.begin(115200);

  pinMode(STATION_LED, OUTPUT);
  pinMode(ACCESSPOINT_LED, OUTPUT);

  pinMode(BUTTON, INPUT_PULLUP);

  //stepsPerRevolution = pSettings->getStepsPerRevolution();
  //maxSpeed = pSettings->getMaxSpeed();
  //direction = pSettings->getDirection();
  //motorInterfaceType = pSettings->getMotorInterfaceType();

  //myStepper = AccelStepper(motorInterfaceType, motorPin1, motorPin3, motorPin2, motorPin4);

  myStepper.setMaxSpeed(maxSpeed);
  myStepper.setSpeed(0);

}

void initServer()
{
  server.close();
  // start webserver

  server.on("/help/", handleHelp);

  // handles notFound
  server.onNotFound(handleHelp);

  // interactive pages
  //server.on("/device/", handleDevice);  deprecated as from version 0.2.0
  server.on("/spin/", handleSpin);
  server.on("/wifi/", handleWiFi);
  // handles input from interactive pages
  server.on("/networkssid/", handleNetworkSSID);
  server.on("/wifiConnect/", handleWifiConnect);
  server.on("/spinSettings/", handleSpinSettings);
  server.on("/language/", handleLanguage);

  // data handler
  server.on("/data.sse/", handleSse);

  server.on("/setRoleModel/", handleRoleModel);
  // url-commands, not used in normal circumstances
  server.on("/ap/", switchToAccessPoint);
  server.on("/network/", switchToNetwork);
  server.on("/deviceSettings/", handleDeviceSettings);
  server.on("/eraseSettings/", eraseSettings);
  server.on("/initSettings/", initSettings);
  server.on("/getSettings/", getSettings);
  server.on("/saveSettings/", saveSettings);
  server.on("/reset/", resetWiFiManagerToFactoryDefaults);
  server.on("/update/", handleVersion);
  server.on("/restart/", handleRestart);

  // handles debug
  server.on("/debug/", mydebug);

  // handles a check if this url is available
  // remove this when clients are availabe
  server.on("/_mdns/", getMDNS);
  server.on("/_ip/", getMyIP);

  server.begin();
  Serial.println("HTTP server started");
}



void setup()
{
  /* It seems to help preventing ESPerror messages with mode(3,6) when
  using a delay */
  initHardware();

   // see https://forum.arduino.cc/index.php?topic=121654.0 voor circuit brownout
  delay(pSettings->WAIT_PERIOD);
  // use EITHER setupWiFi OR setupWiFiManager
  
   // get saved setting from EEPROM
  eepromStartModeAP = pSettings->beginAsAccessPoint();

  if (pSettings->beginAsAccessPoint())
  {
    setupWiFi();        // local network as access point
  }
  else
  {
    setupWiFiManager();   // part of local network as station
  }

  delay(pSettings->WAIT_PERIOD);
  startmDNS();

  delay(pSettings->WAIT_PERIOD);

  initServer();

  // for asyncrequest
  lastSendMillis = millis();

  delay(pSettings->WAIT_PERIOD);

  buttonInterruptOn();
}

void loop()
{
  // update should be run on every loop
  MDNS.update();

  if (detectUpdateFlag == true)
  {
    String result = updateFirmware("latest");
    detectUpdateFlag = false;
  }

  if (updateSucceeded == true)
  {
    ESP.restart();
  }

  if (detectButtonFlag == true)
  {
    toggleWiFi();   // only toggle between AP and STA by using the button and set corresponding Setting
    detectButtonFlag = false;
    // set eepromStartMode to the correct value, needed for independent spinning in ap-mode
    // saving in EEPROM
    if (pSettings->beginAsAccessPoint() == true)
    {
      eepromStartModeAP = true; 
    }
    else
    {
      eepromStartModeAP = false;
    }
    pSettings->saveConfigurationSettings();
  }

  // For ESP8266WebServer
  server.handleClient();

  // For handleHTTPClient
  if (WiFi.getMode() == WIFI_STA)
  {
    /* send data to target server using ESP8266HTTPClient */
    if (millis() - lastSendMillis > pSettings->getSEND_PERIOD())
    {
      if ((aRequest.readyState() == 0) || (aRequest.readyState() == 4)) {
          sendDataToTarget(&aRequest, wifiClient, pSettings, pWifiSettings, String(WiFi.macAddress()), detectInfoRequest);
          detectInfoRequest = false;    // reset value so no info will be sent again
      }
      lastSendMillis = millis();
    }

    String response = getAsyncResponse(&aRequest);
    if (response != "") 
    {
      processServerData(response);
    }
  }

  // For automatic Reset after loosing WiFi connection in STA mode
  if ((WiFi.getMode() == WIFI_AP) && (eepromStartModeAP == false))
  {
    if (no_sta_counter < NO_STA_COUNTER_MAX)
    {
      no_sta_counter +=1;
      delay(50);          // small value because loop must continue for other purposes
    }
    else {
      no_sta_counter = 0;
      setupWiFiManager();  // try to start WiFi again
    }
  }

  // Stepper motor
  if (motorSpeedStepper > 0) {

    myStepper.setSpeed(motorSpeedStepper * direction);
  }
  else
  {
    myStepper.setSpeed(0);
  }
  myStepper.runSpeed();

}
