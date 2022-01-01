#ifndef SETTINGS_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define SETTINGS_H
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <EEPROM.h>               // used to store and read settings

class Settings
{
public:
  /* time between two sse pushes in ms */
  const uint16_t SSE_RETRY = 1000;

  /* wait period in milliseconds */
  uint32_t WAIT_PERIOD = 200;

private:
  /* 4 bytes to store, version of this firmware */
  uint8_t major = 0;   // max 2^8 = 256
  uint8_t minor = 1;   // max 2^8 = 256
  uint16_t patch = 4;  // max 2^16 = 65536

  /* start as Access Point or as Network client */
  bool startAsAccessPoint = false;

  /* factoryStartAsAccessPoint is false */
  bool factoryStartAsAccessPoint = false;

  /* interval for sending data to the target server */
  uint16_t SEND_PERIOD = 5000;

  /* target server, max size = 32 */
  String targetServer = "http://meandmy.info";
  /* factoryTargetServer */
  String factoryTargetServer = "http://meandmy.info";

  /* target port */
  uint16_t targetPort = 80;
  /* factoryTargetPort server */
  uint16_t factoryTargetPort = 80;

  /* target path, max size = 16 */
  String targetPath = "/eat/";
  /* factoryTargetPath server */
  String factoryTargetPath = "/eat/";

  /* Maximum size of EEPROM, SPI_FLASH_SEC_SIZE comes from spi_flash.h */
  const uint16_t MAX_EEPROM_SIZE = SPI_FLASH_SEC_SIZE;
  
  /* first address for Settings storage */
  const uint16_t address = 0;

  /* first available address for Settings storage, for use in other functions or classes */
  uint16_t addressOffset = 0;

  /* check for first saved initialization */
  const uint8_t INITCHECK = 64;

  /* 1 byte to store, holds check for first initialization */
  uint8_t initNumber = 0;
  /* 1 byte to store, factory setting, holds check for first initialization */
  uint8_t factoryInitNumber = 0;

  /* 37 bytes to store, together with the MAC address, the identification of a device */
  String deviceKey = "88888888-4444-4444-4444-121212121212";
  /* 37 bytes to store, factory setting, together with the MAC address, the identification of a device */
  String factoryDeviceKey = "88888888-4444-4444-4444-121212121212";

  /* sizeof of serialized variable, marked as 'to store' */
  uint16_t storageSize;

  /* 3 bytes to store, language */
  String language = "NL";

  /* 3 bytes to store, factory settings for language */
  String factoryLanguage = "NL";

  /* 17 bytes to store, last given IP number to the device from a WiFi network, will not be saved */
  String lastNetworkIP = "Unknown";

  /* roleModel, this is where the model gets the data from, max size = 32 */
  String roleModel = "None";
  /* factoryRoleModel, this is where the model gets the data from, max size = 32 */
  String factoryRoleModel = "None";  // None means no roleModel defined

  /* motor properties */
  int16_t stepsPerRevolution = 4076; // change this in the database to fit the number of steps per revolution
  int16_t maxSpeed = 1000;           // each motortype has its own maximum speed
  int8_t direction = 1;              // 1 or -1; some motors are wired reversed
  uint8_t motorInterfaceType = 8;    // AccelStepper::HALF4WIRE;
  /* factory settings for motor_properties */
  int16_t factoryStepsPerRevolution = 4076; // change this in the database to fit the number of steps per revolution
  int16_t factoryMaxSpeed = 1000;           // each motortype has its own maximum speed
  int8_t factoryDirection = 1;              // 1 or -1; some motors are wired reversed
  uint8_t factoryMotorInterfaceType = 8;    // AccelStepper::HALF4WIRE;


public:
  Settings()
  {
    this->storageSize = sizeof(this->initNumber) + 
                        sizeof(this->major) + 
                        sizeof(this->minor) + 
                        sizeof(this->patch) + 
                        3 +                   // language (NL) + 1
                        sizeof(this->startAsAccessPoint) +
                        33 +                  // max size targetServer + 1
                        sizeof(this->targetPort) + 
                        17 +                  // max size of targetPath + 1
                        33 +                  // max size roleModel + 1
                        sizeof(this->stepsPerRevolution) +
                        sizeof(this->maxSpeed) +
                        sizeof(this->direction) +
                        sizeof(this->motorInterfaceType) +
                        37;                   // MAX_DEVICEKEY + 1

    //this->initSettings(); // is called through the browser
    /* set new address offset */
    /*
    this->setOffsetAddress(this->storageSize); is the same as:
        this->addressOffset = this->address + this->storageSize;
    */
    this->addressOffset = this->address + this->storageSize;
    this->setupEEPROM();
    this->setupUpdatedFirmware();
  };

  ~Settings()
  {
  };

private:
  /* converts a string of numbers to an integer */
  uint8_t atoi8_t(String s);

  /* converts a string of numbers to an integer */
  uint16_t atoi16_t(String s);

  /* converts a string of numbers to an unsigned 32 bits number */
  uint32_t atoi32_t(String s);

  /* check to see if the EEPROM settings are already there */
  bool isInitialized();

  /* check to see if the Firmware has been updated */
  bool isUpdated();

  /* checks for new update, using the version number and sets the new version number */
  uint16_t setupUpdatedFirmware();

public:

  /* get version number, used for firmware updates */
  String getFirmwareVersion();

  /* does the initial setup of the settings and saves the values on EEPROM-address (default start= 0), returns length of saved bytes */
  uint16_t setupEEPROM();

  /* saves settings in EEPROM starting on EEPROM-address (default = 0), returns length of saved bytes */
  uint16_t saveSettings();

  /* erase settings, set value ff on every EEPROM Settings address, returns true if it succeeds */
  bool eraseSettings();

  /* set Settings value to factory values and saves the values on EEPROM-address (default start= 0), returns length of saved bytes */
  uint16_t initSettings();

  /* get Settings from EEPROM */
  uint16_t getSettings();

  /* saves only Changed Configuration Settings in EEPROM starting on EEPROM-address (default = 0), returns length of saved bytes */
  uint16_t saveConfigurationSettings();

  /* return deviceKey */
  String getDeviceKey();

  /* set deviceKey without saving it to EEPROM */
  void setDeviceKey(String myDeviceKey);

  /* period for sending data to the target server */
  uint16_t getSEND_PERIOD();

  /* return factory setting for targetServer */
  String getFactoryTargetServer();

  /* targetServer */
  String getTargetServer();

  /* set target server */
  void setTargetServer(String targetServer);

  /* return factory setting for targetPort */
  uint16_t getFactoryTargetPort();

  /* targetPort */
  uint16_t getTargetPort();

  /* set target port */
  void setTargetPort(String port);

  /* return factory setting for targetPath */
  String getFactoryTargetPath();

  /* targetPath */
  String getTargetPath();

  /* set target path */
  void setTargetPath(String targetPath);

  /* return factory setting for roleModel */
  String getFactoryRoleModel();

  /* roleModel */
  String getRoleModel();

  /* set roleModel */
  void setRoleModel(String roleModel);

  /* get factory setting for stepsPerRevolution */
  int16_t getFactoryStepsPerRevolution();

  /* get factory setting for the motors maximum speed */
  int16_t getFactoryMaxSpeed();

  /* get factory setting for the motor direction */
  int8_t getFactoryDirection();

  /* get factory setting for the motor interface type (AccelStepper) */
  uint8_t getFactoryMotorInterfaceType();

  /* get setting for stepsPerRevolution */
  int16_t getStepsPerRevolution();

  /* get setting for the motors maximum speed */
  int16_t getMaxSpeed();

  /* get setting for the motor direction */
  int8_t getDirection();

  /* get setting for the motor interface type (AccelStepper) */
  uint8_t getMotorInterfaceType();

  /* set setting for stepsPerRevolution */
  void setStepsPerRevolution(int16_t stepsPerRevolution);

  /* set setting for the motors maximum speed */
  void setMaxSpeed(uint16_t maxSpeed);

  /* set setting for the motor direction */
  void setDirection(int8_t direction);

  /* set setting for the motor interface type (AccelStepper) */
  void setMotorInterfaceType(uint8_t motorInterfaceType);

  /* saves settings for motor properties */
  uint16_t saveMotorSettings();

  /* EEPROM Offset Address, for use in other functions or classes */
  uint16_t getOffsetAddress();

  /* EEPROM set new value for Offset Address, for use in other functions or classes */
  bool setOffsetAddress(uint16_t deltaAddress);

  /* returns factory setting beginAs AccessPoint for WiFi start-mode, translated to "ap" or "network" */
  String getFactoryStartModeWiFi();

  /* return start as Access point or as network client */
  bool beginAsAccessPoint();

  /* set start as Access point or as network client */
  void beginAsAccessPoint(bool beginAsAccessPointValue);


  /* language, automatically saved */
  void setLanguage(String language);

  /* language, automatically saved */
  String getLanguage();

  /* network station last known IP address, will not be saved saved */
  void setLastNetworkIP(String lastNetworkIP);

  /* network station last known IP address, will not be saved */
  String getLastNetworkIP();

  /* debug */
  String getMemoryContent(uint16_t start, uint16_t end);
};
#endif