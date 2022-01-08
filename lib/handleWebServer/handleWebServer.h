#ifndef HANDLEWEBSERVER_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define HANDLEWEBSERVER_H
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include "settings.h"
#include "WiFiSettings.h"

/* show the help page */
void help(ESP8266WebServer &server, Settings * pSettings);
void help_nl(ESP8266WebServer &server, Settings * pSettings);
/* choose spinning */
void spin(ESP8266WebServer &server, Settings * pSettings);
void spin_nl(ESP8266WebServer &server, Settings * pSettings);
/* show info mode */
void info(ESP8266WebServer &server, Settings * pSettings, WiFiSettings * pWifiSettings);
void info_nl(ESP8266WebServer &server, Settings * pSettings, WiFiSettings * pWifiSettings);
/* choose wifi connection, (Access Point or Station -todo: get SSID for Station-) */
void wifi(ESP8266WebServer &server, Settings * pSettings, WiFiSettings * pWifiSettings);
void wifi_nl(ESP8266WebServer &server, Settings * pSettings, WiFiSettings * pWifiSettings);
/* sending data through sse */
// not language dependent
void sse(ESP8266WebServer &server, Settings * pSettings, uint32_t revolutions, uint32_t viewPulsesPerMinute);

#endif