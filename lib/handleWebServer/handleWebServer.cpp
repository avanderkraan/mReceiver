#include "handleWebServer.h"

void info(ESP8266WebServer &server, Settings * pSettings, WiFiSettings * pWifiSettings) {
  String starthtml = "<!DOCTYPE HTML>\r\n<html>\r\n";
  starthtml += "<head>\r\n";
  starthtml += "<meta charset=\"utf-8\">\r\n";
  starthtml += "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\r\n";
  starthtml += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  starthtml += "<!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->\r\n";
  starthtml += "<link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgo='>\r\n";
  starthtml += "<title>info</title>\r\n";
  starthtml += "</head>\r\n";
  starthtml += "<body>\r\n";
  String endhtml = "</body>\r\n";
  endhtml += "</html>\r\n";

  String result = starthtml;
  result += "Information about the mill model\r\n";
  result += "<br><br><br>\r\n";
  String myIP = "";
  if (WiFi.getMode() == WIFI_AP)
  {
    myIP = WiFi.softAPIP().toString();
  }
  if (WiFi.getMode() == WIFI_STA)
  {
    myIP = WiFi.localIP().toString();

    result += "Passwords and IP addresses are not sent to the server<br>\r\n";
    result += "Information with bold characters which you see has been sent to ";
    result += pSettings->getTargetServer();     
  }

  result += "\r\n\r\n<br><br>IP address: ";
  result += myIP;

  result += "\r\n<br><strong>Firmware version: ";
  result += pSettings->getFirmwareVersion();
  result += "</strong>";

  result += "\r\n<br><strong>Motor settings:";
  result += "\r\n<br>- Steps per revolution: ";
  result += pSettings->getStepsPerRevolution();
  result += "\r\n<br>- Maximum speed: ";
  result += pSettings->getMaxSpeed();
  result += "\r\n<br>- Direction (1 of -1): ";
  result += pSettings->getDirection();
  result += "\r\n<br>- Interface type: ";
  result += pSettings->getMotorInterfaceType();
  result += "</strong>";
 
  result += "\r\n<br>Access Point SSID: ";
  result += pWifiSettings->readAccessPointSSID();

  result += "\r\n<br>Network SSID: ";
  result += pWifiSettings->readNetworkSSID();

  result += "\r\n<br>Server name: ";
  result += pSettings->getTargetServer();

  result += "\r\n<br>Server port: ";
  result += String(pSettings->getTargetPort());

  result += "\r\n<br>Server path: ";
  result += pSettings->getTargetPath();

  result += "<br>\r\n";
  result += "<br>\r\n";
  result += "<a href='/help/'>Go to the home/help page</a>\r\n";

  result += endhtml;

  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/html", result); 
}

void help(ESP8266WebServer &server, Settings * pSettings)
{
  String result = "<!DOCTYPE HTML>\r\n<html>\r\n";
  result += "<head>\r\n";
  result += "<meta charset=\"utf-8\">\r\n";
  result += "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\r\n";
  result += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  result += "<!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->\r\n";
  result += "<link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgo='>\r\n";
  result += "<title>model</title>\r\n";
  result += "</head>\r\n";
  result += "<body>\r\n";
  result += "Settings for the mill model\r\n";
  result += "<br><br><br>\r\n";
  result += "<input id=\"EN\" type=\"button\" onclick=\"selectLanguage(this)\" value=\"English\">\r\n";
  result += "<input id=\"NL\" type=\"button\" onclick=\"selectLanguage(this)\" value=\"Nederlands\">\r\n";
  result += "<br><br>\r\n";
  result += "WiFi mode: ";
  if (pSettings->beginAsAccessPoint() == true)
  {
    result += "Access Point\r\n";
    result += "<br>\r\n";
    result += "(url: <a href='http://model.local/' target='_blank'>model.local</a> or <a href='http://192.168.4.1/' target='_blank'>http://192.168.4.1</a>)\r\n";
  }
  else
  {
    result += "Network Station\r\n";
    result += "<br>\r\n";
    result += "(url: <a href='http://model.local/' target='_blank'>model.local</a> or via a local IP address, last known is: <a href='http://";
    result += pSettings->getLastNetworkIP();
    result += "/' target='_blank'>";
    result += pSettings->getLastNetworkIP();
    result += "</a>\r\n";
    result += ")\r\n";
  }  
  result += "<br>\r\n";
  result += "<div id=\"sendMessage\"></div>\r\n";
  result += "<br><br>\r\n";

  result += "<input id='restartButton' type='button' onclick='restart()' value='Restart'<br>\r\n";
  result += "<br><br>\r\n";

  result += "Version: <span id='version'></span>\r\n";
  result += " <input id='updateFirmwareButton' type='button' onclick='updateFirmware()' value='Update Firmware'<br>\r\n";
  result += " <div id=\"updateFirmwareMessage\"><div>\r\n";
  result += "<br><br>\r\n";

  result += "Menu\r\n";
  result += "<br><br><br>\r\n";
  result += "<a href='/help/'>help</a> help/home screen\r\n";
  result += "<br><br>\r\n";
  result += "<a href='/spin/'>Model spin settings</a> Set speed or connect a real mill\r\n";
  result += "<br><br>\r\n";
  result += "<a href='/wifi/'>WiFi</a> settings to connect the Model to WiFi\r\n";
  result += "<br><br>\r\n";
  result += "<a href='/info/'>Information</a> shown on screen and partially sent to the server. Passwords and IP addresses are not sent<br>\r\n";
  result += "<br><br>\r\n";

  result += "<script>\r\n";
  result += "  function restart() {\r\n";
  result += "    document.getElementById(\"restartButton\").disabled = true\r\n";
  result += "    document.getElementById(\"updateFirmwareButton\").disabled = true\r\n";
  result += "    document.getElementById(\"updateFirmwareMessage\").innerHTML = \"Please refresh this page after about 1 minute\"\r\n";
  result += "    var params = \"name=restart\";\r\n";
  result += "    sendUpdateFirmware(params, \"/restart/\");\r\n";
  result += "  };\r\n";
  result += "</script>\r\n";

  result += "<script>\r\n";
  result += "  document.getElementById(\"version\").innerHTML = \"";
  result += pSettings->getFirmwareVersion();
  result += "\";\r\n";
  result += "  function updateFirmware() {\r\n";
  result += "    document.getElementById(\"restartButton\").disabled = true\r\n";
  result += "    document.getElementById(\"updateFirmwareButton\").disabled = true\r\n";
  result += "    document.getElementById(\"updateFirmwareMessage\").innerHTML = \"Please refresh this page after about 1 minute\"\r\n";
  result += "    var params = \"name=update\";\r\n";
  result += "    sendUpdateFirmware(params, \"/update/\");\r\n";
  result += "  };\r\n";
  result += "</script>\r\n";

  result += "<script>\r\n";
  result += "  function selectLanguage(component) {\r\n";
  result += "    var params = \"name=help\" + \"&language=\" + component.id;\r\n";
  result += "    document.getElementById(\"NL\").disabled = true;\r\n";
  result += "    document.getElementById(\"EN\").disabled = true;\r\n";
  result += "    sendData(params, \"/language/\");\r\n";
  result += "  }\r\n";
  result += "  function sendData(data, path) {\r\n";
  result += "    var xhr = new XMLHttpRequest();   // new HttpRequest instance\r\n";
  result += "    xhr.open(\"POST\", path);\r\n";
  result += "    xhr.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\r\n";
  result += "    //xhr.setRequestHeader(\"Content-Type\", \"application/json\");\r\n";
  result += "   document.getElementById(\"sendMessage\").innerHTML = \"Please wait\";\r\n";
  result += "    xhr.onreadystatechange = function() { // Call a function when the state changes.\r\n";
  result += "     var myResponseText = \"\";\r\n";
  result += "      if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {\r\n";
  result += "        window.location.reload();\r\n";
  //result += "       myResponseText = this.responseText || \"\";\r\n";
  result += "     }\r\n";
  result += "      if (this.readyState === XMLHttpRequest.DONE && this.status !== 200) {\r\n";
  result += "       myResponseText = this.statusText || \"\";\r\n";
  result += "      }\r\n";
  result += "      document.getElementById(\"sendMessage\").innerHTML = myResponseText;\r\n";
  result += "    }\r\n";
  result += "    xhr.send(data);\r\n";
  result += "  }\r\n";

  result += "  function sendUpdateFirmware(data, path) {\r\n";
  result += "    var xhr = new XMLHttpRequest();   // new HttpRequest instance\r\n";
  result += "    xhr.open(\"POST\", path);\r\n";
  result += "    xhr.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\r\n";
  result += "   document.getElementById(\"sendMessage\").innerHTML = \"Please wait\";\r\n";
  result += "    xhr.onreadystatechange = function() { // Call a function when the state changes.\r\n";
  result += "     var myResponseText = \"\";\r\n";
  result += "      if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {\r\n";
  result += "        window.location.reload();\r\n";
  //result += "       myResponseText = this.responseText || \"\";\r\n";
  result += "     }\r\n";
  result += "      if (this.readyState === XMLHttpRequest.DONE && this.status !== 200) {\r\n";
  result += "       myResponseText = this.statusText || \"\";\r\n";
  result += "      }\r\n";
  result += "      document.getElementById(\"sendMessage\").innerHTML = myResponseText;\r\n";
  result += "    }\r\n";
  result += "    xhr.send(data);\r\n";
  result += "  }\r\n";

  result += "</script>\r\n";

  result += "\r\n</body>\r\n</html>\r\n";
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/html", result);
}

void spin_nl(ESP8266WebServer &server, Settings * pSettings)
{
  String result = "<!DOCTYPE HTML><html>\r\n";
  result += "<head>\r\n";
  result += "<meta charset=\"utf-8\">\r\n";
  result += "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\r\n";
  result += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  result += "<!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->\r\n";
  result += "<link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgo='>\r\n";
  result += "<title>model</title>\r\n";
  result += "</head>\r\n";
  result += "<body>\r\n";
  result += "Draai Instellingen voor het molen model\r\n";
  result += "<br><br><br>\r\n";
  result += "<div id=\"spinSetting\" onclick=\"clearMessage();\">\r\n";
  result += "    Dit model kan zelf draaien maar kan ook gekoppeld zijn aan een echte molen \r\n";
  result += "    <br>\r\n";
  result += "<input type=\"radio\" name=\"spin\" onclick=\"displaySpinSetting()\" value=\"independent\" ";
  result += (pSettings->getRoleModel() == "independent")?"checked":"";
  result += "> Zelfstandig\r\n";
  result += "<br>\r\n";
  result += "<input type=\"radio\" name=\"spin\" onclick=\"displaySpinSetting()\" value=\"connected\" ";
  result += ((pSettings->getRoleModel() != "independent") && (pSettings->getRoleModel() != "None"))?"checked":"";
  result += "> Gekoppeld aan een molen\r\n";
  result += "<br>\r\n";
  result += "<input type=\"radio\" name=\"spin\" onclick=\"displaySpinSetting()\" value=\"stop\" ";
  result += (pSettings->getRoleModel() == "None")?"checked":"";
  result += "> Stop\r\n";
  result += "<br>\r\n";
  result += "<br>\r\n";

  result += "<div id=\"spinSettingSpeed\">\r\n";
  result += "Geef een snelheid op (0-";
  result += pSettings->getMaxRoleModelRPM();
  result += " toeren per minuut)\r\n";
  result += "<br>\r\n";
  result += " Speed <input type=\"text\" name=\"roleModelSpeed\" min=\"0\" max=\"";
  result += pSettings->getMaxRoleModelRPM();
  result += "\" maxlength=\"2\" size=\"3\" onkeyup=\"checkNumber(this, 'speedMessage', 'Invalid number (0 - ";
  result += pSettings->getMaxRoleModelRPM();
  result += "), changed to ";
  result += pSettings->getMaxRoleModelRPM();
  result += "');\" placeholder=\"0\" value=\"";
  result += pSettings->getMaxRoleModelRPM();
  result += "\"";
  result += "\"> Max 2 karakters\r\n";
  result += "<div id=\"speedMessage\"></div>\r\n";
  result += "</div>\r\n";

  result += "<div id=\"spinSettingCode\">\r\n";
  result += "Geef een molencode\r\n";
  result += "<br>\r\n";
  result += "Ga voor een lijst naar \r\n";
  if (WiFi.getMode() == WIFI_STA)
  {
    result += "<a href=\"";
    result += pSettings->getTargetServer();
    if ((pSettings->getTargetPort() != 80) && (pSettings->getTargetPort() != 443))
    {
      result += ":";
      result += String(pSettings->getTargetPort());
    }
    result += "/codes/?lang=nl";
    result += "\" target=\"_blank\">\r\n";
  }
  result += pSettings->getTargetServer();
  if ((pSettings->getTargetPort() != 80) && (pSettings->getTargetPort() != 443))
  {
    result += ":";
    result += String(pSettings->getTargetPort());
  }
  result += "/codes/?lang=nl";
  if (WiFi.getMode() == WIFI_STA)
  {
    result += "</a>\r\n";
  } 
  result += "<br>\r\n";
  result += " Code <input type=\"text\" name=\"roleModelCode\" maxlength=\"32\" size=\"33\" placeholder=\"01234\" value=\"";
  if (pSettings->getRoleModel() != "independent")
  {
    result += pSettings->getRoleModel();
  }
  result += "\"> Max 32 karakters\r\n";
  result += "</div>\r\n";

  result += "</div>\r\n";

  result += "<br><br>\r\n";
  result += "  Na 'Save' even geduld tot er een bevestiging is.\r\n";
  result += "<br>\r\n";
  result += "<input id=\"spinButton\" type=\"button\" name=\"spinButton\" value=\"Save\" onclick=\"saveSpinSetting()\">\r\n";
  result += "<input type=\"button\" name=\"spinCancelButton\" value=\"Cancel\" onclick=\"cancelSettings()\">\r\n";

  result += "\r\n";
  result += "<br>\r\n";
  result += "<div id=\"sendMessage\"></div>\r\n";

  result += "<br>\r\n";
  result += "<br>\r\n";
  result += "<a href='/help/'>Ga naar de begin/help pagina</a>\r\n";
  result += "<script>\r\n";

  result += "function clearMessage() {\r\n";
  result += "  document.getElementById(\"sendMessage\").innerHTML = \"\";\r\n";
  result += "}\r\n";

  result += "  function checkNumber(component, messageId, message) {\r\n";
  result += "  //var validCharacterString = \"0123456789-.\";\r\n";
  result += "  var valid = false;\r\n";
  result += "    if ((component.value >= Number(component.getAttribute(\"min\"))) && (component.value <= Number(component.getAttribute(\"max\")))) {\r\n";
  result += "        valid = true;\r\n";
  result += "    }\r\n";
  result += "    if (valid) {\r\n";
  result += "      document.getElementById(messageId).innerHTML = \"\";\r\n";
  result += "    }\r\n";
  result += "    else {\r\n";
  result += "      document.getElementById(messageId).innerHTML = message;\r\n";
  result += "      component.value = component.getAttribute(\"max\");\r\n";
  result += "    }\r\n";
  result += "    return valid;\r\n";
  result += "  }\r\n";
  result += "\r\n";

  result += "  function displaySpinSetting() {\r\n";
  result += "    var spinMode = \"\"\r\n";
  result += "    var ele = document.getElementsByName(\"spin\") || [];\r\n";
  result += "    for(i = 0; i < ele.length; i++) {\r\n";
  result += "      if(ele[i].checked) {\r\n";
  result += "        spinMode = ele[i].value;\r\n";
  result += "      }\r\n";
  result += "    }\r\n";
  result += "    if (spinMode == \"independent\") {\r\n";
  result += "        document.getElementById(\"spinSettingSpeed\").style.display=\"block\";\r\n";
  result += "        document.getElementById(\"spinSettingCode\").style.display=\"none\";\r\n";
  result += "    }\r\n";
  result += "    else {\r\n";
  result += "        document.getElementById(\"spinSettingSpeed\").style.display=\"none\";\r\n";
  result += "        document.getElementById(\"spinSettingCode\").style.display=\"block\";\r\n";
  result += "    }\r\n";
  result += "    if (spinMode == \"stop\") {\r\n";
  result += "        document.getElementById(\"spinSettingSpeed\").style.display=\"none\";\r\n";
  result += "        document.getElementById(\"spinSettingCode\").style.display=\"none\";\r\n";
  result += "    };\r\n";
  result += "}\r\n";

  result += "  function cancelSettings() {\r\n";
  result += "    window.location.reload();\r\n";
  result += "  }\r\n";
  result += "  function sendData(data) {\r\n";
  result += "    var xhr = new XMLHttpRequest();   // new HttpRequest instance\r\n";
  result += "    xhr.open(\"POST\", \"/spinSettings/\");\r\n";
  result += "    xhr.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\r\n";
  result += "    //xhr.setRequestHeader(\"Content-Type\", \"application/json\");\r\n";
  result += "      document.getElementById(\"sendMessage\").innerHTML = \"Please wait\";\r\n";
  result += "      xhr.onreadystatechange = function() { // Call a function when the state changes.\r\n";
  result += "        var myResponseText = \"\";\r\n";
  result += "        if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {\r\n";
  result += "          myResponseText = this.responseText || \"\";\r\n";
  result += "        }\r\n";
  result += "        if (this.readyState === XMLHttpRequest.DONE && this.status !== 200) {\r\n";
  result += "          myResponseText = this.statusText || \"\";\r\n";
  result += "        }\r\n";
  result += "        document.getElementById(\"sendMessage\").innerHTML = myResponseText;\r\n";
  result += "      }\r\n";
  //result += "    }\r\n";
  result += "    xhr.send(data);\r\n";
  result += "  }\r\n";
  result += "\r\n";
  result += "  function saveSpinSetting() {\r\n";
  result += "    var ele = document.getElementsByName(\"spin\");\r\n";
  result += "    var spinmode = \"\"\r\n";
  result += "    for(i = 0; i < ele.length; i++) {\r\n";
  result += "      if(ele[i].checked) {\r\n";
  result += "        spinMode = ele[i].value;\r\n";
  result += "      }\r\n";
  result += "    }\r\n";
  result += "    var roleModelSpeed = document.getElementsByName(\"roleModelSpeed\")[0].value || \"\";\r\n";
  result += "    var roleModelCode = document.getElementsByName(\"roleModelCode\")[0].value || \"\";\r\n";
  result += "    var params = \"name=spin\" + \"&spinMode=\" + spinMode + \"&roleModelSpeed=\" + roleModelSpeed + \"&roleModelCode=\" + roleModelCode;\r\n";
  result += "    sendData(params);\r\n";
  result += "  }\r\n";
  result += "displaySpinSetting();\r\n";
  result += "</script>\r\n";

  result += "\r\n";
  result += "</body>\r\n";
  result += "</html>\r\n";
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/html", result);
}

void spin(ESP8266WebServer &server, Settings * pSettings)
{
  String result = "<!DOCTYPE HTML><html>\r\n";
  result += "<head>\r\n";
  result += "<meta charset=\"utf-8\">\r\n";
  result += "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\r\n";
  result += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  result += "<!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->\r\n";
  result += "<link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgo='>\r\n";
  result += "<title>model</title>\r\n";
  result += "</head>\r\n";
  result += "<body>\r\n";
  result += "Spin Settings for the mill model\r\n";
  result += "<br><br><br>\r\n";
  result += "<div id=\"spinSetting\" onclick=\"clearMessage();\">\r\n";
  result += "    This model can spin by itself and also be connected to a real mill \r\n";
  result += "    <br>\r\n";
  result += "<input type=\"radio\" name=\"spin\" onclick=\"displaySpinSetting()\" value=\"independent\" ";
  result += (pSettings->getRoleModel() == "independent")?"checked":"";
  result += "> Independent\r\n";
  result += "<br>\r\n";
  result += "<input type=\"radio\" name=\"spin\" onclick=\"displaySpinSetting()\" value=\"connected\" ";
  result += ((pSettings->getRoleModel() != "independent") && (pSettings->getRoleModel() != "None"))?"checked":"";
  result += "> Connect to a mill\r\n";
  result += "<br>\r\n";
  result += "<input type=\"radio\" name=\"spin\" onclick=\"displaySpinSetting()\" value=\"stop\" ";
  result += (pSettings->getRoleModel() == "None")?"checked":"";
  result += "> Stop\r\n";
  result += "<br>\r\n";
  result += "<br>\r\n";

  result += "<div id=\"spinSettingSpeed\">\r\n";
  result += "Enter a speed (0-";
  result += pSettings->getMaxRoleModelRPM();
  result += " revolutions per minute)\r\n";
  result += "<br>\r\n";
  result += " Speed <input type=\"text\" name=\"roleModelSpeed\" min=\"0\" max=\"";
  result += pSettings->getMaxRoleModelRPM();
  result += "\" maxlength=\"2\" size=\"3\" onkeyup=\"checkNumber(this, 'speedMessage', 'Invalid number (0 - ";
  result += pSettings->getMaxRoleModelRPM();
  result += "), changed to ";
  result += pSettings->getMaxRoleModelRPM();
  result += "');\" placeholder=\"0\" value=\"";
  result += pSettings->getMaxRoleModelRPM();
  result += "\"";
  result += "\"> Max 2 characters\r\n";
  result += "<div id=\"speedMessage\"></div>\r\n";
  result += "</div>\r\n";

  result += "<div id=\"spinSettingCode\">\r\n";
  result += "Enter a millcode\r\n";
  result += "<br>\r\n";
  result += "Find a list on \r\n";
  if (WiFi.getMode() == WIFI_STA)
  {
    result += "<a href=\"";
    result += pSettings->getTargetServer();
    if ((pSettings->getTargetPort() != 80) && (pSettings->getTargetPort() != 443))
    {
      result += ":";
      result += String(pSettings->getTargetPort());
    }
    result += "/codes/?lang=en";
    result += "\" target=\"_blank\">\r\n";
  }
  result += pSettings->getTargetServer();
  if ((pSettings->getTargetPort() != 80) && (pSettings->getTargetPort() != 443))
  {
    result += ":";
    result += String(pSettings->getTargetPort());
  }
  result += "/codes/?lang=en";
  if (WiFi.getMode() == WIFI_STA)
  {
    result += "</a>\r\n";
  } 
  result += "<br>\r\n";
  result += " Code <input type=\"text\" name=\"roleModelCode\" maxlength=\"32\" size=\"33\" placeholder=\"01234\" value=\"";
  if (pSettings->getRoleModel() != "independent")
  {
    result += pSettings->getRoleModel();
  }
  result += "\"> Max 32 characters\r\n";
  result += "</div>\r\n";

  result += "</div>\r\n";

  result += "<br><br>\r\n";
  result += "After 'Save' wait for confirmation.\r\n";
  result += "<br>\r\n";
  result += "<input id=\"spinButton\" type=\"button\" name=\"spinButton\" value=\"Save\" onclick=\"saveSpinSetting()\">\r\n";
  result += "<input type=\"button\" name=\"spinCancelButton\" value=\"Cancel\" onclick=\"cancelSettings()\">\r\n";

  result += "\r\n";
  result += "<br>\r\n";
  result += "<div id=\"sendMessage\"></div>\r\n";

  result += "<br>\r\n";
  result += "<br>\r\n";
  result += "<a href='/help/'>Go to the home/help page</a>\r\n";
  result += "<script>\r\n";

  result += "function clearMessage() {\r\n";
  result += "  document.getElementById(\"sendMessage\").innerHTML = \"\";\r\n";
  result += "}\r\n";

  result += "  function checkNumber(component, messageId, message) {\r\n";
  result += "  //var validCharacterString = \"0123456789-.\";\r\n";
  result += "  var valid = false;\r\n";
  result += "    if ((component.value >= Number(component.getAttribute(\"min\"))) && (component.value <= Number(component.getAttribute(\"max\")))) {\r\n";
  result += "        valid = true;\r\n";
  result += "    }\r\n";
  result += "    if (valid) {\r\n";
  result += "      document.getElementById(messageId).innerHTML = \"\";\r\n";
  result += "    }\r\n";
  result += "    else {\r\n";
  result += "      document.getElementById(messageId).innerHTML = message;\r\n";
  result += "      component.value = component.getAttribute(\"max\");\r\n";
  result += "    }\r\n";
  result += "    return valid;\r\n";
  result += "  }\r\n";
  result += "\r\n";

  result += "  function displaySpinSetting() {\r\n";
  result += "    var spinMode = \"\"\r\n";
  result += "    var ele = document.getElementsByName(\"spin\") || [];\r\n";
  result += "    for(i = 0; i < ele.length; i++) {\r\n";
  result += "      if(ele[i].checked) {\r\n";
  result += "        spinMode = ele[i].value;\r\n";
  result += "      }\r\n";
  result += "    }\r\n";
  result += "    if (spinMode == \"independent\") {\r\n";
  result += "        document.getElementById(\"spinSettingSpeed\").style.display=\"block\";\r\n";
  result += "        document.getElementById(\"spinSettingCode\").style.display=\"none\";\r\n";
  result += "    }\r\n";
  result += "    else {\r\n";
  result += "        document.getElementById(\"spinSettingSpeed\").style.display=\"none\";\r\n";
  result += "        document.getElementById(\"spinSettingCode\").style.display=\"block\";\r\n";
  result += "    }\r\n";
  result += "    if (spinMode == \"stop\") {\r\n";
  result += "        document.getElementById(\"spinSettingSpeed\").style.display=\"none\";\r\n";
  result += "        document.getElementById(\"spinSettingCode\").style.display=\"none\";\r\n";
  result += "    };\r\n";
  result += "}\r\n";

  result += "  function cancelSettings() {\r\n";
  result += "    window.location.reload();\r\n";
  result += "  }\r\n";
  result += "  function sendData(data) {\r\n";
  result += "    var xhr = new XMLHttpRequest();   // new HttpRequest instance\r\n";
  result += "    xhr.open(\"POST\", \"/spinSettings/\");\r\n";
  result += "    xhr.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\r\n";
  result += "    //xhr.setRequestHeader(\"Content-Type\", \"application/json\");\r\n";
  result += "      document.getElementById(\"sendMessage\").innerHTML = \"Please wait\";\r\n";
  result += "      xhr.onreadystatechange = function() { // Call a function when the state changes.\r\n";
  result += "        var myResponseText = \"\";\r\n";
  result += "        if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {\r\n";
  result += "          myResponseText = this.responseText || \"\";\r\n";
  result += "        }\r\n";
  result += "        if (this.readyState === XMLHttpRequest.DONE && this.status !== 200) {\r\n";
  result += "          myResponseText = this.statusText || \"\";\r\n";
  result += "        }\r\n";
  result += "        document.getElementById(\"sendMessage\").innerHTML = myResponseText;\r\n";
  result += "      }\r\n";
  //result += "    }\r\n";
  result += "    xhr.send(data);\r\n";
  result += "  }\r\n";
  result += "\r\n";
  result += "  function saveSpinSetting() {\r\n";
  result += "    var ele = document.getElementsByName(\"spin\");\r\n";
  result += "    var spinmode = \"\"\r\n";
  result += "    for(i = 0; i < ele.length; i++) {\r\n";
  result += "      if(ele[i].checked) {\r\n";
  result += "        spinMode = ele[i].value;\r\n";
  result += "      }\r\n";
  result += "    }\r\n";
  result += "    var roleModelSpeed = document.getElementsByName(\"roleModelSpeed\")[0].value || \"\";\r\n";
  result += "    var roleModelCode = document.getElementsByName(\"roleModelCode\")[0].value || \"\";\r\n";
  result += "    var params = \"name=spin\" + \"&spinMode=\" + spinMode + \"&roleModelSpeed=\" + roleModelSpeed + \"&roleModelCode=\" + roleModelCode;\r\n";
  result += "    sendData(params);\r\n";
  result += "  }\r\n";
  result += "displaySpinSetting();\r\n";
  result += "</script>\r\n";

  result += "\r\n";
  result += "</body>\r\n";
  result += "</html>\r\n";
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/html", result);
}

void wifi(ESP8266WebServer &server, Settings * pSettings, WiFiSettings * pWiFiSettings)
{
  String result = "<!DOCTYPE HTML>\r\n<html>\r\n";
  result += "<head>\r\n";
  result += "<meta charset=\"utf-8\">\r\n";
  result += "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\r\n";
  result += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  result += "<!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->\r\n";
  result += "<link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgo='>\r\n";
  result += "<title>model</title>\r\n";
  result += "</head>\r\n";
  result += "<body>\r\n";
  result += "WiFi settings for the mill model\r\n";
  result += "<br><br><br>\r\n";
  result += "Choose AccessPoint to change the default password\r\n";
  result += "<br>\r\n";
  result += "Choose Network to connect the model to your own WiFi\r\n";
  result += "<br><br>\r\n";
  result += "<input type=\"radio\" name=\"wifi\" onclick=\"displayWiFiMode()\" value=\"ap\">Access Point\r\n";
  result += "<br>\r\n";
  result += "<input type=\"radio\" name=\"wifi\" onclick=\"displayWiFiMode()\" value=\"network\">Network Station\r\n";
  result += "<br>\r\n";
  result += "<input type=\"radio\" name=\"wifi\" onclick=\"displayWiFiMode()\" value=\"erase\">Go to Erase-WiFi-Data menu\r\n";
  result += "<br>\r\n";
  result += "<br>\r\n";
  result += "<div id=\"ap\">\r\n";
  result += "  Clients can get access to this Access Point using the SSID and password entered below\r\n";
  result += "  <br>\r\n";
  //result += "  An empty SSID will result in a default SSID for the Model\r\n";
  //result += "  <br>\r\n";
  result += "  An empty password will result in an unencrypted, open Access Point\r\n";
  result += "  <br>\r\n";
  //result += "  SSID: <input type=\"text\" name=\"ssid\" maxlength=\"32\" size=\"33\" placeholder=\"";
  result += "  SSID: ";
  //result += "  SSID: <input type=\"text\" name=\"ssid\" maxlength=\"32\" size=\"33\" placeholder=\"";
  if (pWiFiSettings->getAccessPointSSID() == "")
  {
    result += "ESP-" + WiFi.softAPmacAddress();
  }
  else
  {
    result += pWiFiSettings->getAccessPointSSID();
  }
  //result += "\" value=\"";
  //result += pWiFiSettings->getAccessPointSSID();
  //result += "\">\r\n";
  result += "  <br>\r\n";
  result += "  Password: <input type=\"password\" name=\"password\" maxlength=\"32\" size=\"33\" placeholder=\"";
  if (pWiFiSettings->getAccessPointPassword() == "")
  {
    result += "administrator";
  }
  else
  {
    result += pWiFiSettings->getAccessPointPassword();
  }
  result += "\" value=\"";
  result += pWiFiSettings->getAccessPointPassword();
  result += "\" onkeyup=\"checkURIComponent(this, 'apPasswordMessage', 'Invalid password character');\">\r\n";
  result += " <span id=\"apPasswordMessage\"></span>\r\n";
  result += "  <br><br>\r\n";
  result += "  After 'Save' wait for confirmation.\r\n";
  result += "  <br>\r\n";
  result += "  <input id=\"apButton\" type=\"button\" name=\"apButton\" value=\"Save\" onclick=\"saveAP(this)\">\r\n";
  result += "  <input type=\"button\" name=\"apCancelButton\" value=\"Cancel\" onclick=\"cancelWiFi()\">\r\n";
  result += "</div>\r\n";
  result += "\r\n";
  result += "<div id=\"network\">\r\n";
  result += "  Connect to one of the WiFi networks below\r\n";
  result += "  <br><br>\r\n";
  result += "  <span id=\"ssidList\">Please wait for a list or a message to show here</span>\r\n";
  result += "  <br><br>\r\n";
  result += "  <span id=\"selectedWiFiPassword\">";
  result += "Give the password for the selected WiFi network <span id=\"selectedWiFi\"></span>\r\n";
  result += "     <br>\r\n";
  result += "     Password: <input type=\"password\" name=\"password\" maxlength=\"32\" size=\"33\" placeholder=\"password\" value=\"";
  result += "\" onkeyup=\"checkURIComponent(this, 'networkPasswordMessage', 'Invalid password character');\">\r\n";
  result += "     <br><br>\r\n";
  result += "  </span>\r\n";
  result += "  After 'Save' wait for confirmation.\r\n";
  result += "  <br>\r\n";
  result += "   <input id=\"networkButton\" type=\"button\" name=\"networkButton\" value=\"Save\" onclick=\"saveNetwork(this)\">\r\n";
  result += "   <input type=\"button\" name=\"networkCancelButton\" value=\"Cancel\" onclick=\"cancelWiFi()\">\r\n";
  result += "  <span id=\"networkPasswordMessage\"></span>\r\n";
  result += "</div>\r\n";
  result += "<div id=\"erase\">\r\n";
  result += "  !!!Warning!!!\r\n";
  result += "  <br><br>\r\n";
  result += "  A click on an Erase button will immediately erase saved data! Your current connection will stay alive.\r\n";
  result += "  <br><br>\r\n";
  result += "Do NOT Erase and <input type=\"button\" name=\"networkButton\" value=\"Cancel\" onclick=\"cancelWiFi()\">\r\n";
  result += "  <br><br>\r\n";
  result += "  <input id=\"eraseAPData\" type=\"button\" name=\"eraseAPDataButton\" value=\"Erase\" onclick=\"eraseWiFiData(this)\"> Erase Access Point data, will result in an unencrypted, open Access Point\r\n";
  result += "  <br>\r\n";
  result += "  <input id=\"eraseNetworkData\" type=\"button\" name=\"eraseNetworkDataButton\" value=\"Erase\" onclick=\"eraseWiFiData(this)\"> Erase Network data\r\n";
  result += "  <br>\r\n";
  result += "  <input id=\"eraseWiFiData\" type=\"button\" name=\"eraseWiFiDataButton\" value=\"Erase\" onclick=\"eraseWiFiData(this)\"> Erase Access Point AND Network data\r\n";
  result += "</div>\r\n";
  result += "\r\n";
  result += "  <br>\r\n";
  result += "<div id=\"sendMessage\"></div>\r\n";
  result += "\r\n";
  result += "<br>\r\n";
  result += "<br>\r\n";
  result += "<a href='/help/'>Go to the home/help page</a>\r\n";
  result += "<script>\r\n";
  result += "function checkURIComponent(component, messageId, message) {\r\n";
  result += "  var invalidCharacterArray = [\" \"]\r\n";
  result += "  var buttonNodeId = component.parentNode.id;\r\n";
  result += "  if (buttonNodeId == \"selectedWiFiPassword\") {\r\n";
  result += "    buttonNodeId = component.parentNode.parentNode.id;\r\n";
  result += "  };\r\n";
  result += "  var buttonId = buttonNodeId + \"Button\"\r\n";
  result += "   var valid = true;\r\n";
  result += "   for (var c = 0; c < invalidCharacterArray.length; c++) {\r\n";
  result += "    if (component.value.indexOf(invalidCharacterArray[c]) > -1) {\r\n";
  result += "      valid = false;\r\n";
  result += "      break;\r\n";
  result += "    };\r\n";
  result += "  };\r\n";
  result += "  if (valid) {\r\n";
  result += "    document.getElementById(messageId).innerHTML = \"\";\r\n";
  result += "  }\r\n";
  result += "  else {\r\n";
  result += "    document.getElementById(messageId).innerHTML = message;\r\n";
  result += "  }\r\n";
  result += "  document.getElementById(buttonId).disabled = !valid;\r\n";
  result += "   return valid;\r\n";
  result += "}\r\n";
  result += "  function cancelWiFi() {\r\n";
  result += "    window.location.reload();\r\n";
  result += "  }\r\n";
  result += "  function sendData(data) {\r\n";
  result += "    var xhr = new XMLHttpRequest();   // new HttpRequest instance\r\n";
  result += "    xhr.open(\"POST\", \"/wifiConnect/\");\r\n";
  result += "    xhr.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\r\n";
  result += "    //xhr.setRequestHeader(\"Content-Type\", \"application/json\");\r\n";
  result += "   document.getElementById(\"sendMessage\").innerHTML = \"Please wait\";\r\n";
  result += "    xhr.onreadystatechange = function() { // Call a function when the state changes.\r\n";
  result += "     var myResponseText = \"\";\r\n";
  result += "      if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {\r\n";
  result += "       myResponseText = this.responseText || \"\";\r\n";
  result += "     }\r\n";
  result += "      if (this.readyState === XMLHttpRequest.DONE && this.status !== 200) {\r\n";
  result += "       myResponseText = this.statusText || \"\";\r\n";
  result += "      }\r\n";
  result += "      document.getElementById(\"sendMessage\").innerHTML = myResponseText;\r\n";
  result += "    }\r\n";
  result += "    xhr.send(data);\r\n";
  result += "  }\r\n";
  result += "\r\n";
  result += "  function saveAP(content) {\r\n";
  result += "    var children = content.parentNode.childNodes;\r\n";
  result += "    var ssid = \"\";\r\n";
  result += "    var password = \"\";\r\n";
  result += "    for (var i = 0; i < children.length; i++) {\r\n";
  result += "      if (children[i].name == \"ssid\") {\r\n";
  result += "        ssid = children[i].value || \"\";\r\n";
  result += "      }\r\n";
  result += "      if (children[i].name == \"password\") {\r\n";
  result += "        password = children[i].value || \"\";\r\n";
  result += "      }\r\n";
  result += "    }\r\n";
  result += "    var params = \"name=ap\" + \"&ssid=\" + encodeURIComponent(ssid) + \"&password=\" + encodeURIComponent(password);\r\n";
  result += "    sendData(params);\r\n";
  result += "    //var json_upload = JSON.stringify({name:\"ap\", ssid:ssid, password:password});\r\n";
  result += "    //sendData(json_upload);\r\n";
  result += "  }\r\n";
  result += "\r\n";
  result += "  function saveNetwork(content) {\r\n";
  result += "    var children = document.getElementById(\"selectedWiFiPassword\").childNodes;\r\n";
  result += "    var ssid = document.getElementById(\"selectedWiFi\").innerHTML;\r\n";
  result += "    var password = \"\";\r\n";
  result += "    for (var i = 0; i < children.length; i++) {\r\n";
  result += "      if (children[i].name == \"password\") {\r\n";
  result += "        password = children[i].value || \"\";\r\n";
  result += "      }\r\n";
  result += "    }\r\n";
  result += "    var params = \"name=network\" + \"&ssid=\" + encodeURIComponent(ssid) + \"&password=\" + encodeURIComponent(password);\r\n";
  result += "    sendData(params);\r\n";
  result += "    //var json_upload = JSON.stringify({name:\"network\", ssid:ssid, password:password});\r\n";
  result += "    //sendData(json_upload);\r\n";
  result += "  }\r\n";
  result += "  function eraseWiFiData(content) {\r\n";
  result += "  var params = \"name=erase\" + \"&target=\" + content.id;\r\n";
  result += "      sendData(params);\r\n";
  result += "  }\r\n";
  result += "</script>\r\n";
  result += "\r\n";
  result += "<script>\r\n";

  result += "function showNetworkList(networks) {\r\n";
  result += "  var showNetworkList = [];\r\n";
  result += "  var showNetwork = \"\";\r\n";
  result += "  var networkList = (new Function(\"return [\" + networks + \"];\")());\r\n";
  result += "  networkList.sort((a,b) => (a.dBm > b.dBm) ? 1: (a.dBm === b.dBm) ? ((a.ssid > b.ssid) ? 1: -1) : -1);\r\n";
  result += "  networkList = networkList.filter(function(e) { return e.dBm > -80});\r\n";
  result += "  sortedList = [...networkList].reverse();\r\n";
  result += "  for (var key in sortedList) {\r\n";
  result += "    var value = sortedList[key]\r\n";
  result += "    showNetworkList.push(Object.values(value)[0]);\r\n";
  result += "  }\r\n";
  result += "  var uniqueShowNetworkList = [...new Set(showNetworkList)];\r\n";
  result += "  for (var key in uniqueShowNetworkList) {\r\n";
  result += "    showNetwork += \"<div>\" + uniqueShowNetworkList[key] + \"</div>\";\r\n";
  result += "  }\r\n";
  result += "  if (showNetwork == \"\") {\r\n";
  result += "    showNetwork = \"No WiFi signal available\"\r\n";
  result += "  }\r\n";
  result += "  else {\r\n";
  result += "    document.getElementById(\"selectedWiFiPassword\").style.display=\"block\"\r\n";
  result += "  }\r\n";
  result += "  document.getElementById(\"ssidList\").innerHTML = showNetwork;\r\n";
  result += "}\r\n"; 
  result += "function selectNetworkSSID(content) {\r\n";
  result += "  ssid = \"\"\r\n";
  result += "  if (content.name == \"networkSSID\") {\r\n";
  result += "    ssid = content.value;\r\n";
  result += "  }\r\n";
  result += "  document.getElementById(\"selectedWiFi\").innerHTML = ssid;\r\n";
  result += "}\r\n";
  result += "function loadWiFiNetworkList() {\r\n";
  result += "  var xhttp = new XMLHttpRequest();\r\n";
  result += "  xhttp.onreadystatechange = function() {\r\n";
  result += "    if (this.readyState == 4 && this.status == 200) {\r\n";
  result += "      showNetworkList(this.responseText);\r\n";
  result += "    }\r\n";
  result += "  };\r\n";
  result += "  xhttp.open(\"GET\", \"/networkssid/\", true);\r\n";
  result += "  xhttp.send();\r\n";
  result += "}\r\n";
  result += "</script>\r\n";
  result += "\r\n";
  result += "<script>\r\n";
  result += "function displayWiFiMode() {\r\n";
  result += "var ele = document.getElementsByName('wifi');\r\n";
  result += "\r\n";
  result += "  for(i = 0; i < ele.length; i++) {\r\n";
  result += "    if(ele[i].checked) {\r\n";
  result += "       document.getElementById(ele[i].value).style.display=\"block\";\r\n";
  result += "       if (ele[i].value == 'network') {\r\n";
  result += "        document.getElementById(\"selectedWiFi\").innerHTML = \"\"\r\n";
  result += "        document.getElementById(\"selectedWiFiPassword\").style.display=\"none\"\r\n";
  result += "         loadWiFiNetworkList();\r\n";
  result += "       }\r\n";
  result += "     }\r\n";
  result += "     else {\r\n";
  result += "       document.getElementById(ele[i].value).style.display=\"none\";\r\n";
  result += "     }\r\n";
  result += "  }\r\n";
  result += "}\r\n";
  result += "displayWiFiMode();\r\n";
  result += "</script>\r\n";
  result += "</body>\r\n";
  result += "</html>\r\n";
  
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/html", result);
}

///////////////// sse is language independent ///////////////////////////
void sse(ESP8266WebServer &server, Settings * pSettings, uint32_t revolutions, uint32_t viewPulsesPerMinute)
{
  String result = "retry: ";
  result += String(pSettings->SSE_RETRY);
  result += "\r\n";
  result += "data: ";
  result += "{";
  result += "\"revolutions\":";
  result += String(revolutions);
  result += ",";
  result += "\"viewPulsesPerMinute\":";
  result += String(viewPulsesPerMinute);
  result += "}\r\n\r\n";
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/event-stream", result);

}

///////////////// Vanaf hier Nederlands ///////////////////////////
void info_nl(ESP8266WebServer &server, Settings * pSettings, WiFiSettings * pWifiSettings) {
  String starthtml = "<!DOCTYPE HTML>\r\n<html>\r\n";
  starthtml += "<head>\r\n";
  starthtml += "<meta charset=\"utf-8\">\r\n";
  starthtml += "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\r\n";
  starthtml += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  starthtml += "<!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->\r\n";
  starthtml += "<link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgo='>\r\n";
  starthtml += "<title>info</title>\r\n";
  starthtml += "</head>\r\n";
  starthtml += "<body>\r\n";
  String endhtml = "</body>\r\n";
  endhtml += "</html>\r\n";

  String result = starthtml;
  result += "Informatie over het molen model\r\n";
  result += "<br><br><br>\r\n";

  String myIP = "";
  if (WiFi.getMode() == WIFI_AP)
  {
    myIP = WiFi.softAPIP().toString();
  }
  if (WiFi.getMode() == WIFI_STA)
  {
    myIP = WiFi.localIP().toString();

    result += "Wachtwoorden en IP adressen worden niet naar de server gestuurd<br>\r\n";
    result += "Informatie met dikgedrukte letters die je hieronder ziet is opgestuurd naar ";
    result += pSettings->getTargetServer();     
  }

  result += "\r\n\r\n<br><br>IP adres: ";
  result += myIP;

  result += "\r\n<br><strong>Firmware versie: ";
  result += pSettings->getFirmwareVersion();
  result += "</strong>";

  result += "\r\n<br><strong>Motor instellingen: ";
  result += "\r\n<br>- Stappen per omwenteling: ";
  result += pSettings->getStepsPerRevolution();
  result += "\r\n<br>- Maximale snelheid: ";
  result += pSettings->getMaxSpeed();
  result += "\r\n<br>- Richting (1 of -1): ";
  result += pSettings->getDirection();
  result += "\r\n<br>- Interface type: ";
  result += pSettings->getMotorInterfaceType();
  result += "</strong>";

  result += "\r\n<br>Access Point SSID: ";
  result += pWifiSettings->readAccessPointSSID();

  result += "\r\n<br>Netwerk SSID: ";
  result += pWifiSettings->readNetworkSSID();

  result += "\r\n<br>Server naam: ";
  result += pSettings->getTargetServer();

  result += "\r\n<br>Server poort: ";
  result += String(pSettings->getTargetPort());

  result += "\r\n<br>Server pad: ";
  result += pSettings->getTargetPath();

  result += "<br>\r\n";
  result += "<br>\r\n";
  result += "<a href='/help/'>Ga naar de begin/help pagina</a>\r\n";

  result += endhtml;

  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/html", result); 
}

void help_nl(ESP8266WebServer &server, Settings * pSettings)
{
  String result = "<!DOCTYPE HTML>\r\n<html>\r\n";
  result += "<head>\r\n";
  result += "<meta charset=\"utf-8\">\r\n";
  result += "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\r\n";
  result += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  result += "<!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->\r\n";
  result += "<link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgo='>\r\n";
  result += "<title>model</title>\r\n";
  result += "</head>\r\n";
  result += "<body>\r\n";
  result += "Instellingen voor het molen model\r\n";
  result += "<br><br><br>\r\n";
  result += "<input id=\"EN\" type=\"button\" onclick=\"selectLanguage(this)\" value=\"English\">\r\n";
  result += "<input id=\"NL\" type=\"button\" onclick=\"selectLanguage(this)\" value=\"Nederlands\">\r\n";
  result += "<br><br>\r\n";
  result += "WiFi modus: ";
  if (pSettings->beginAsAccessPoint() == true)
  {
    result += "Access Point\r\n";
    result += "<br>\r\n";
    result += "(url: <a href='http://model.local/' target='_blank'>model.local</a> of <a href='http://192.168.4.1/' target='_blank'>http://192.168.4.1</a>)\r\n";
  }
  else
  {
    result += "Netwerk Station\r\n";
    result += "<br>\r\n";
    result += "(url: <a href='http://model.local/' target='_blank'>model.local</a> of via een lokaal IP adres, laatst bekende adres is: <a href='http://";
    result += pSettings->getLastNetworkIP();
    result += "/' target='_blank'>";
    result += pSettings->getLastNetworkIP();
    result += "</a>\r\n";
    result += ")\r\n";
  }  
  result += "<br>\r\n";
  result += "<div id=\"sendMessage\"></div>\r\n";
  result += "<br><br>\r\n";

  result += "<input id='restartButton' type='button' onclick='restart()' value='Restart'<br>\r\n";
  result += "<br><br>\r\n";

  result += "Versie: <span id='version'></span>\r\n";
  result += " <input id='updateFirmwareButton' type='button' onclick='updateFirmware()' value='Update Firmware'<br>\r\n";
  result += " <div id=\"updateFirmwareMessage\"><div>\r\n";
  result += "<br><br>\r\n";

  result += "Menu\n";
  result += "<br><br><br>\r\n";
  result += "<a href='/help/'>help</a> begin/help scherm\r\n";
  result += "<br><br>\r\n";
  result += "<a href='/spin/'>Model draai instellingen</a> Geeft een snelheid of koppel aan een echte molen\r\n";
  result += "<br><br>\r\n";
  result += "<a href='/wifi/'>WiFi</a> instellingen om het Model te koppelen aan WiFi\r\n";
  result += "<br><br>\r\n";
  result += "<a href='/info/'>Informatie</a> op het scherm, wordt deels verzonden naar de server. Wachtwoorden en IP adressen worden niet verstuurd<br>\r\n";
  result += "<br><br>\r\n";

  result += "<script>\r\n";
  result += "  function restart() {\r\n";
  result += "    document.getElementById(\"restartButton\").disabled = true\r\n";
  result += "    document.getElementById(\"updateFirmwareButton\").disabled = true\r\n";
  result += "    document.getElementById(\"updateFirmwareMessage\").innerHTML = \"Please refresh this page after about 1 minute\"\r\n";
  result += "    var params = \"name=restart\";\r\n";
  result += "    sendUpdateFirmware(params, \"/restart/\");\r\n";
  result += "  };\r\n";
  result += "</script>\r\n";

  result += "<script>\r\n";
  result += "  document.getElementById(\"version\").innerHTML = \"";
  result += pSettings->getFirmwareVersion();
  result += "\";\r\n";
  result += "  function updateFirmware() {\r\n";
  result += "    document.getElementById(\"restartButton\").disabled = true\r\n";
  result += "    document.getElementById(\"updateFirmwareButton\").disabled = true\r\n";
  result += "    document.getElementById(\"updateFirmwareMessage\").innerHTML = \"Na ongeveer 1 minuut kun je de pagina verversen\"\r\n";
  result += "    var params = \"name=update\";\r\n";
  result += "    sendUpdateFirmware(params, \"/update/\");\r\n";
  result += "  };\r\n";
  result += "</script>\r\n";

  result += "<script>\r\n";
  result += "  function selectLanguage(component) {\r\n";
  result += "    var params = \"name=help\" + \"&language=\" + component.id;\r\n";
  result += "    document.getElementById(\"NL\").disabled = true;\r\n";
  result += "    document.getElementById(\"EN\").disabled = true;\r\n";
  result += "    sendData(params, \"/language/\");\r\n";
  result += "  }\r\n";
  result += "  function sendData(data, path) {\r\n";
  result += "    var xhr = new XMLHttpRequest();   // new HttpRequest instance\r\n";
  result += "    xhr.open(\"POST\", path);\r\n";
  result += "    xhr.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\r\n";
  result += "    //xhr.setRequestHeader(\"Content-Type\", \"application/json\");\r\n";
  result += "    document.getElementById(\"sendMessage\").innerHTML = \"Please wait\";\r\n";
  result += "    xhr.onreadystatechange = function() { // Call a function when the state changes.\r\n";
  result += "      var myResponseText = \"\";\r\n";
  result += "        if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {\r\n";
  result += "          window.location.reload();\r\n";
  result += "        }\r\n";
  result += "      if (this.readyState === XMLHttpRequest.DONE && this.status !== 200) {\r\n";
  result += "       myResponseText = this.statusText || \"\";\r\n";
  result += "      }\r\n";
  result += "      document.getElementById(\"sendMessage\").innerHTML = myResponseText;\r\n";
  result += "    }\r\n";
  result += "    xhr.send(data);\r\n";
  result += "  }\r\n";

  result += "  function sendUpdateFirmware(data, path) {\r\n";
  result += "    var xhr = new XMLHttpRequest();   // new HttpRequest instance\r\n";
  result += "    xhr.open(\"POST\", path);\r\n";
  result += "    xhr.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\r\n";
  result += "   document.getElementById(\"sendMessage\").innerHTML = \"Please wait\";\r\n";
  result += "    xhr.onreadystatechange = function() { // Call a function when the state changes.\r\n";
  result += "     var myResponseText = \"\";\r\n";
  result += "      if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {\r\n";
  result += "        window.location.reload();\r\n";
  //result += "       myResponseText = this.responseText || \"\";\r\n";
  result += "     }\r\n";
  result += "      if (this.readyState === XMLHttpRequest.DONE && this.status !== 200) {\r\n";
  result += "       myResponseText = this.statusText || \"\";\r\n";
  result += "      }\r\n";
  result += "      document.getElementById(\"sendMessage\").innerHTML = myResponseText;\r\n";
  result += "    }\r\n";
  result += "    xhr.send(data);\r\n";
  result += "  }\r\n";

  result += "</script>\r\n";
  result += "\r\n</body>\r\n</html>\r\n";
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/html", result);
}

void wifi_nl(ESP8266WebServer &server, Settings * pSettings, WiFiSettings * pWiFiSettings)
{
  String result = "<!DOCTYPE HTML>\r\n<html>\r\n";
  result += "<head>\r\n";
  result += "<meta charset=\"utf-8\">\r\n";
  result += "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\r\n";
  result += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  result += "<!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->\r\n";
  result += "<link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgo='>\r\n";
  result += "<title>model</title>\r\n";
  result += "</head>\r\n";
  result += "<body>\r\n";
  result += "WiFi instellingen voor het molen model\r\n";
  result += "<br><br><br>\r\n";
  result += "Kies AccessPoint om het standaard wachtwoord te wijzigen\r\n";
  result += "<br>\r\n";
  result += "Kies Netwerk Station om het model te koppelen aan je eigen WiFi\r\n";
  result += "<br><br>\r\n";
  result += "<input type=\"radio\" name=\"wifi\" onclick=\"displayWiFiMode()\" value=\"ap\">Access Point\r\n";
  result += "<br>\r\n";
  result += "<input type=\"radio\" name=\"wifi\" onclick=\"displayWiFiMode()\" value=\"network\">Netwerk Station\r\n";
  result += "<br>\r\n";
  result += "<input type=\"radio\" name=\"wifi\" onclick=\"displayWiFiMode()\" value=\"erase\">Ga naar Verwijder WiFi-gegevens menu\r\n";
  result += "<br>\r\n";
  result += "<br>\r\n";
  result += "<div id=\"ap\">\r\n";
  result += "  Apparaten kunnen toegang krijgen tot dit Access Point met het hieronder ingevulde SSID en wachtwoord\r\n";
  result += "  <br>\r\n";
  //result += "  Een niet-ingevuld SSID geeft het standaard SSID van het Access Point\r\n";
  //result += "  <br>\r\n";
  result += "  Een niet-ingevuld wachtwoord geeft een onveilige, open Access Point\r\n";
  result += "  <br>\r\n";
  result += "  SSID: ";
  //result += "  SSID: <input type=\"text\" name=\"ssid\" maxlength=\"32\" size=\"33\" placeholder=\"";
  if (pWiFiSettings->getAccessPointSSID() == "")
  {
    result += "ESP-" + WiFi.softAPmacAddress();
  }
  else
  {
    result += pWiFiSettings->getAccessPointSSID();
  }
  //result += "\" value=\"";
  //result += pWiFiSettings->getAccessPointSSID();
  //result += "\">\r\n";
  result += "  <br>\r\n";
  result += "  Wachtwoord: <input type=\"password\" name=\"password\" maxlength=\"32\" size=\"33\" placeholder=\"";
  if (pWiFiSettings->getAccessPointPassword() == "")
  {
    result += "administrator";
  }
  else
  {
    result += pWiFiSettings->getAccessPointPassword();
  }
  result += "\" value=\"";
  result += pWiFiSettings->getAccessPointPassword();
  result += "\" onkeyup=\"checkURIComponent(this, 'apPasswordMessage', 'Ongeldig wachtwoord karakter');\">\r\n";
  result += " <span id=\"apPasswordMessage\"></span>\r\n";
  result += "  <br><br>\r\n";
  result += "  Na 'Save' even geduld tot er een bevestiging is.\r\n";
  result += "  <br>\r\n";
  result += "  <input id=\"apButton\" type=\"button\" name=\"apButton\" value=\"Save\" onclick=\"saveAP(this)\">\r\n";
  result += "  <input type=\"button\" name=\"apCancelButton\" value=\"Cancel\" onclick=\"cancelWiFi()\">\r\n";
  result += "</div>\r\n";
  result += "\r\n";
  result += "<div id=\"network\">\r\n";
  result += "  Maak verbinding met een van de WiFi netwerken hieronder\r\n";
  result += "  <br><br>\r\n";
  result += "  <span id=\"ssidList\">Wacht even tot er een lijst of een melding verschijnt</span>\r\n";
  result += "  <br><br>\r\n";
  result += "  <span id=\"selectedWiFiPassword\">";
  result += "Geef het wachtwoord voor het geselecteerde WiFi netwerk <span id=\"selectedWiFi\"></span>\r\n";
  result += "     <br>\r\n";
  result += "     Wachtwoord: <input type=\"password\" name=\"password\" maxlength=\"32\" size=\"33\" placeholder=\"password\" value=\"";
  result += "\" onkeyup=\"checkURIComponent(this, 'networkPasswordMessage', 'Ongeldig wachtwoord karakter');\">\r\n";
  result += "     <br><br>\r\n";
  result += "  </span>\r\n";
  result += "  Na 'Save' even geduld tot er een bevestiging is.\r\n";
  result += "  <br>\r\n";
  result += "   <input id=\"networkButton\" type=\"button\" name=\"networkButton\" value=\"Save\" onclick=\"saveNetwork(this)\">\r\n";
  result += "   <input type=\"button\" name=\"networkCancelButton\" value=\"Cancel\" onclick=\"cancelWiFi()\">\r\n";
  result += "  <span id=\"networkPasswordMessage\"></span>\r\n";
  result += "</div>\r\n";
  result += "<div id=\"erase\">\r\n";
  result += "  !!!Opgelet!!!\r\n";
  result += "  <br><br>\r\n";
  result += "  Een klik op de 'Erase' knop zal onmiddellijk de opgeslagen gegevens verwijderen! De huidige verbinding blijft bestaan.\r\n";
  result += "  <br><br>\r\n";
  result += "Verwijder NIETS en <input type=\"button\" name=\"networkButton\" value=\"Cancel\" onclick=\"cancelWiFi()\">\r\n";
  result += "  <br><br>\r\n";
  result += "  <input id=\"eraseAPData\" type=\"button\" name=\"eraseAPDataButton\" value=\"Erase\" onclick=\"eraseWiFiData(this)\"> Verwijderen van Access Point resulteert in in een onveilig, open Access Point\r\n";
  result += "  <br>\r\n";
  result += "  <input id=\"eraseNetworkData\" type=\"button\" name=\"eraseNetworkDataButton\" value=\"Erase\" onclick=\"eraseWiFiData(this)\"> Verwijder Netwerk gegevens\r\n";
  result += "  <br>\r\n";
  result += "  <input id=\"eraseWiFiData\" type=\"button\" name=\"eraseWiFiDataButton\" value=\"Erase\" onclick=\"eraseWiFiData(this)\"> Verwijder Access Point EN Netwerk gegevens\r\n";
  result += "</div>\r\n";
  result += "\r\n";
  result += "  <br>\r\n";
  result += "<div id=\"sendMessage\"></div>\r\n";
  result += "\r\n";
  result += "<br>\r\n";
  result += "<br>\r\n";
  result += "<a href='/help/'>Ga naar de begin/help pagina</a>\r\n";
  result += "<script>\r\n";
  result += "function checkURIComponent(component, messageId, message) {\r\n";
  result += "  var invalidCharacterArray = [\" \"]\r\n";
  result += "  var buttonNodeId = component.parentNode.id;\r\n";
  result += "  if (buttonNodeId == \"selectedWiFiPassword\") {\r\n";
  result += "    buttonNodeId = component.parentNode.parentNode.id;\r\n";
  result += "  };\r\n";
  result += "  var buttonId = buttonNodeId + \"Button\"\r\n";
  result += "   var valid = true;\r\n";
  result += "   for (var c = 0; c < invalidCharacterArray.length; c++) {\r\n";
  result += "    if (component.value.indexOf(invalidCharacterArray[c]) > -1) {\r\n";
  result += "      valid = false;\r\n";
  result += "      break;\r\n";
  result += "    };\r\n";
  result += "  };\r\n";
  result += "  if (valid) {\r\n";
  result += "    document.getElementById(messageId).innerHTML = \"\";\r\n";
  result += "  }\r\n";
  result += "  else {\r\n";
  result += "    document.getElementById(messageId).innerHTML = message;\r\n";
  result += "  }\r\n";
  result += "  document.getElementById(buttonId).disabled = !valid;\r\n";
  result += "   return valid;\r\n";
  result += "}\r\n";
  result += "  function cancelWiFi() {\r\n";
  result += "    window.location.reload();\r\n";
  result += "  }\r\n";
  result += "  function sendData(data) {\r\n";
  result += "    var xhr = new XMLHttpRequest();   // new HttpRequest instance\r\n";
  result += "    xhr.open(\"POST\", \"/wifiConnect/\");\r\n";
  result += "    xhr.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\r\n";
  result += "    //xhr.setRequestHeader(\"Content-Type\", \"application/json\");\r\n";
  result += "   document.getElementById(\"sendMessage\").innerHTML = \"Even geduld\";\r\n";
  result += "    xhr.onreadystatechange = function() { // Call a function when the state changes.\r\n";
  result += "     var myResponseText = \"\";\r\n";
  result += "      if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {\r\n";
  result += "       myResponseText = this.responseText || \"\";\r\n";
  result += "     }\r\n";
  result += "      if (this.readyState === XMLHttpRequest.DONE && this.status !== 200) {\r\n";
  result += "       myResponseText = this.statusText || \"\";\r\n";
  result += "      }\r\n";
  result += "      document.getElementById(\"sendMessage\").innerHTML = myResponseText;\r\n";
  result += "    }\r\n";
  result += "    xhr.send(data);\r\n";
  result += "  }\r\n";
  result += "\r\n";
  result += "  function saveAP(content) {\r\n";
  result += "    var children = content.parentNode.childNodes;\r\n";
  result += "    var ssid = \"\";\r\n";
  result += "    var password = \"\";\r\n";
  result += "    for (var i = 0; i < children.length; i++) {\r\n";
  result += "      if (children[i].name == \"ssid\") {\r\n";
  result += "        ssid = children[i].value || \"\";\r\n";
  result += "      }\r\n";
  result += "      if (children[i].name == \"password\") {\r\n";
  result += "        password = children[i].value || \"\";\r\n";
  result += "      }\r\n";
  result += "    }\r\n";
  result += "    var params = \"name=ap\" + \"&ssid=\" + encodeURIComponent(ssid) + \"&password=\" + encodeURIComponent(password);\r\n";
  result += "    sendData(params);\r\n";
  result += "    //var json_upload = JSON.stringify({name:\"ap\", ssid:ssid, password:password});\r\n";
  result += "    //sendData(json_upload);\r\n";
  result += "  }\r\n";
  result += "\r\n";
  result += "  function saveNetwork(content) {\r\n";
  result += "    var children = document.getElementById(\"selectedWiFiPassword\").childNodes;\r\n";
  result += "    var ssid = document.getElementById(\"selectedWiFi\").innerHTML;\r\n";
  result += "    var password = \"\";\r\n";
  result += "    for (var i = 0; i < children.length; i++) {\r\n";
  result += "      if (children[i].name == \"password\") {\r\n";
  result += "        password = children[i].value || \"\";\r\n";
  result += "      }\r\n";
  result += "    }\r\n";
  result += "    var params = \"name=network\" + \"&ssid=\" + encodeURIComponent(ssid) + \"&password=\" + encodeURIComponent(password);\r\n";
  result += "    sendData(params);\r\n";
  result += "    //var json_upload = JSON.stringify({name:\"network\", ssid:ssid, password:password});\r\n";
  result += "    //sendData(json_upload);\r\n";
  result += "  }\r\n";
  result += "  function eraseWiFiData(content) {\r\n";
  result += "  var params = \"name=erase\" + \"&target=\" + content.id;\r\n";
  result += "      sendData(params);\r\n";
  result += "  }\r\n";
  result += "</script>\r\n";
  result += "\r\n";
  result += "<script>\r\n";

  result += "function showNetworkList(networks) {\r\n";
  result += "  var showNetworkList = [];\r\n";
  result += "  var showNetwork = \"\";\r\n";
  result += "  var networkList = (new Function(\"return [\" + networks + \"];\")());\r\n";
  result += "  networkList.sort((a,b) => (a.dBm > b.dBm) ? 1: (a.dBm === b.dBm) ? ((a.ssid > b.ssid) ? 1: -1) : -1);\r\n";
  result += "  networkList = networkList.filter(function(e) { return e.dBm > -80});\r\n";
  result += "  sortedList = [...networkList].reverse();\r\n";
  result += "  for (var key in sortedList) {\r\n";
  result += "    var value = sortedList[key]\r\n";
  result += "    showNetworkList.push(Object.values(value)[0]);\r\n";
  result += "  }\r\n";
  result += "  var uniqueShowNetworkList = [...new Set(showNetworkList)];\r\n";
  result += "  for (var key in uniqueShowNetworkList) {\r\n";
  result += "    showNetwork += \"<div>\" + uniqueShowNetworkList[key] + \"</div>\";\r\n";
  result += "  }\r\n";
  result += "  if (showNetwork == \"\") {\r\n";
  result += "    showNetwork = \"Geen WiFi signaal beschikbaar\"\r\n";
  result += "  }\r\n";
  result += "  else {\r\n";
  result += "    document.getElementById(\"selectedWiFiPassword\").style.display=\"block\"\r\n";
  result += "  }\r\n";
  result += "  document.getElementById(\"ssidList\").innerHTML = showNetwork;\r\n";
  result += "}\r\n"; 
  result += "function selectNetworkSSID(content) {\r\n";
  result += "  ssid = \"\"\r\n";
  result += "  if (content.name == \"networkSSID\") {\r\n";
  result += "    ssid = content.value;\r\n";
  result += "  }\r\n";
  result += "  document.getElementById(\"selectedWiFi\").innerHTML = ssid;\r\n";
  result += "}\r\n";
  result += "function loadWiFiNetworkList() {\r\n";
  result += "  var xhttp = new XMLHttpRequest();\r\n";
  result += "  xhttp.onreadystatechange = function() {\r\n";
  result += "    if (this.readyState == 4 && this.status == 200) {\r\n";
  result += "      showNetworkList(this.responseText);\r\n";
  result += "    }\r\n";
  result += "  };\r\n";
  result += "  xhttp.open(\"GET\", \"/networkssid/\", true);\r\n";
  result += "  xhttp.send();\r\n";
  result += "}\r\n";
  result += "</script>\r\n";
  result += "\r\n";
  result += "<script>\r\n";
  result += "function displayWiFiMode() {\r\n";
  result += "var ele = document.getElementsByName('wifi');\r\n";
  result += "\r\n";
  result += "  for(i = 0; i < ele.length; i++) {\r\n";
  result += "    if(ele[i].checked) {\r\n";
  result += "       document.getElementById(ele[i].value).style.display=\"block\";\r\n";
  result += "       if (ele[i].value == 'network') {\r\n";
  result += "        document.getElementById(\"selectedWiFi\").innerHTML = \"\"\r\n";
  result += "        document.getElementById(\"selectedWiFiPassword\").style.display=\"none\"\r\n";
  result += "         loadWiFiNetworkList();\r\n";
  result += "       }\r\n";
  result += "     }\r\n";
  result += "     else {\r\n";
  result += "       document.getElementById(ele[i].value).style.display=\"none\";\r\n";
  result += "     }\r\n";
  result += "  }\r\n";
  result += "}\r\n";
  result += "displayWiFiMode();\r\n";
  result += "</script>\r\n";
  result += "</body>\r\n";
  result += "</html>\r\n";
  
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/html", result);
}
