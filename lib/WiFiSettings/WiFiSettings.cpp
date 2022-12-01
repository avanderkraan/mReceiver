#include "WiFiSettings.h"

bool WiFiSettings::isInitialized() {
  /* if first 8 characters are 0xff then it is considered not initialised */
  uint16_t startAddress = this->address;
  uint16_t lastAddress = this->address + 8;
  delay(this->WAIT_PERIOD);

  EEPROM.begin(this->MAX_EEPROM_SIZE);
  bool initialised = false;
  for (uint16_t i = startAddress; i < lastAddress; i++) {
    if ((initialised == false) && (EEPROM.read(i) != 0xff)) {
      initialised = true;
    };
  }

  EEPROM.end();  // release RAM copy of EEPROM content

  delay(this->WAIT_PERIOD);
 
  return initialised;
}

uint16_t WiFiSettings::saveAuthorizationAccessPoint()
{
  uint16_t firstAddress = this->address;
  uint16_t address = this->address;    // Access Point Settings comes first

  if (this->getStorageSizeIsAvailable())
    {
    // The function EEPROM.put() uses EEPROM.update() to perform the write, so does not rewrites the value if it didn't change.
    // It seems to help preventing ESPerror messages with mode(3,6) when using a delay 
    delay(this->WAIT_PERIOD);

    EEPROM.begin(this->MAX_EEPROM_SIZE);

    char myssidAccessPoint[129];  // one more for the null character
    strcpy(myssidAccessPoint, this->ssidAccessPoint.c_str());
    EEPROM.put(address, myssidAccessPoint);
    address += 129;

    char myPasswordAccessPoint[129];  // one more for the null character
    strcpy(myPasswordAccessPoint, this->passwordAccessPoint.c_str());
    EEPROM.put(address, myPasswordAccessPoint);
    address += 129;
  
    EEPROM.commit();    // with success it will return true
    EEPROM.end();       // release RAM copy of EEPROM content

    delay(this->WAIT_PERIOD);
    }

  return address - firstAddress;
}

uint16_t WiFiSettings::saveAuthorizationNetwork()
{
  uint16_t firstAddress = this->address;
  uint16_t address = this->address + 258;  // Network Settings comes after Access Point Settings

  if (this->getStorageSizeIsAvailable())
    {
    // The function EEPROM.put() uses EEPROM.update() to perform the write, so does not rewrites the value if it didn't change.
    // It seems to help preventing ESPerror messages with mode(3,6) when using a delay 
    delay(this->WAIT_PERIOD);

    EEPROM.begin(this->MAX_EEPROM_SIZE);

    char myssidNetwork[129];  // one more for the null character
    strcpy(myssidNetwork, this->ssidNetwork.c_str());
    EEPROM.put(address, myssidNetwork);
    address += 129;

    char myPasswordNetwork[129];  // one more for the null character
    strcpy(myPasswordNetwork, this->passwordNetwork.c_str());
    EEPROM.put(address, myPasswordNetwork);
    address += 129;
  
    EEPROM.commit();    // with success it will return true
    EEPROM.end();       // release RAM copy of EEPROM content

    delay(this->WAIT_PERIOD);
    }

  return address - firstAddress;
}

bool WiFiSettings::getStorageSizeIsAvailable()
{
    return this->storageSizeIsAvailable;
}

bool WiFiSettings::eraseSettings(uint16_t startAddress, uint16_t size) {
  if (this->getStorageSizeIsAvailable())
  {
    uint16_t lastAddress = startAddress + size;
    delay(this->WAIT_PERIOD);

    EEPROM.begin(this->MAX_EEPROM_SIZE);
    // replace values in EEPROM with 0xff
    for (uint16_t i = startAddress; i < lastAddress; i++) {
      EEPROM.write(i,0xff);
    }
    bool result = EEPROM.commit();    // with success it will return true
    EEPROM.end();  // release RAM copy of EEPROM content

    delay(this->WAIT_PERIOD);
    return result;
  }
  return false;
}

bool WiFiSettings::eraseWiFiSettings() {
  if (this->eraseSettings(this->address, this->storageSize))
  {
    this->passwordAccessPoint = "";
    this->ssidAccessPoint = "";
    this->passwordNetwork = "";
    this->ssidNetwork = "";
    return true;
  }
  return false;
}

bool WiFiSettings::eraseAccessPointSettings() {
  if (this->eraseSettings(this->address, 258))
  {
    this->passwordAccessPoint = "";
    this->ssidAccessPoint = "";
    return true;
  }
  return false;
}

bool WiFiSettings::eraseNetworkSettings() {
  if (this->eraseSettings(this->address + 258, 258))
  {
    this->passwordNetwork = "";
    this->ssidNetwork = "";
    return true;
  }
  return false;
}

void WiFiSettings::setAccessPointSSID(String ssid)
{
    this->ssidAccessPoint = ssid;
}

String WiFiSettings::getAccessPointSSID()
{
    return this->ssidAccessPoint;
}

void WiFiSettings::setAccessPointPassword(String password)
{
    this->passwordAccessPoint = password;
}

String WiFiSettings::getAccessPointPassword()
{
    return this->passwordAccessPoint;
}

void WiFiSettings::setNetworkSSID(String ssid)
{
    this->ssidNetwork = ssid;
}

String WiFiSettings::getNetworkSSID()
{
    return this->ssidNetwork;
}

void WiFiSettings::setNetworkPassword(String password)
{
    this->passwordNetwork = this->dencrypt.encrypt(password);
}

String WiFiSettings::getNetworkPassword()
{
    return this->dencrypt.decrypt(this->passwordNetwork);
}

String WiFiSettings::readAccessPointSSID()
{
  if (this->getStorageSizeIsAvailable())
  {
    uint16_t address = this->address ;  // Access Point SSID Setting comes first
    char myAccessPointSSID[129];

    EEPROM.begin(this->MAX_EEPROM_SIZE);
    EEPROM.get(address, myAccessPointSSID);
    EEPROM.end();  // release RAM copy of EEPROM content
    delay(this->WAIT_PERIOD);
    if (myAccessPointSSID[0] == 0xff) {
      return "";
    }    
    return String(myAccessPointSSID);
  }
  return "";
}

String WiFiSettings::readAccessPointPassword()
{
  if (this->getStorageSizeIsAvailable())
  {
    uint16_t address = this->address + 129;  // Access Point Password Setting comes second
    char myAccessPointPassword[129];

    EEPROM.begin(this->MAX_EEPROM_SIZE);
    EEPROM.get(address, myAccessPointPassword);
    EEPROM.end();  // release RAM copy of EEPROM content
    delay(this->WAIT_PERIOD);
    if (myAccessPointPassword[0] == 0xff) {
      return "";
    }
    return String(myAccessPointPassword);
  }
  return "";
}

String WiFiSettings::readNetworkSSID()
{  
  if (this->getStorageSizeIsAvailable())
  {
    uint16_t address = this->address + 258;  // Network SSID Setting comes third
    char myNetworkSSID[129];

    EEPROM.begin(this->MAX_EEPROM_SIZE);
    EEPROM.get(address, myNetworkSSID);
    EEPROM.end();  // release RAM copy of EEPROM content
    delay(this->WAIT_PERIOD);
    return String(myNetworkSSID);
  }
  return "";
}

String WiFiSettings::readNetworkPassword()
{  
  if (this->getStorageSizeIsAvailable())
    {
    uint16_t address = this->address + 387;  // Network Password Setting comes fourth
    char myNetworkPassword[129];

    EEPROM.begin(this->MAX_EEPROM_SIZE);
    EEPROM.get(address, myNetworkPassword);
    EEPROM.end();  // release RAM copy of EEPROM content
    delay(this->WAIT_PERIOD);
    return this->dencrypt.decrypt(String(myNetworkPassword));
    }
  return "";
}