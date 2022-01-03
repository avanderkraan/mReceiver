#include "handleHTTPClient.h"
#include "base64.h"

String getSendData(Settings * pSettings, String macAddress) {
  String result = "\"data\": {";
  result += "\"mac\":";      // macAddress
  result += "\"";
  result += macAddress;
  result += "\",";
  result += "\"rM\":";       // roleModel id
  result += "\"";
  result += pSettings->getRoleModel();
  result += "\"";
  result += "}";
  return result;
}

String getSendInfo(Settings * pSettings, WiFiSettings* pWifiSettings, String macAddress) {
  String result = "";
  result += "\"info\": {";

  result += "\"v\":";        // firmwareVersion
  result += "\"";
  result += pSettings->getFirmwareVersion();
  result += "\",";

  result += "\"key\":";      // deviceKey
  result += "\"";
  result += pSettings->getDeviceKey();
  result += "\",";

  result += "\"spr\":";
  result += "\"";
  result += pSettings->getStepsPerRevolution();
  result += "\",";

  result += "\"ms\":";
  result += "\"";
  result += pSettings->getMaxSpeed();
  result += "\",";

  result += "\"d\":";
  result += "\"";
  result += pSettings->getDirection();
  result += "\",";

  result += "\"mit\":";
  result += "\"";
  result += pSettings->getMotorInterfaceType();
  result += "\",";

  result += "\"apssid\":";
  result += "\"";
  result += pWifiSettings->readAccessPointSSID();
  result += "\",";

  result += "\"stssid\":";
  result += "\"";
  result += pWifiSettings->readNetworkSSID();
  result += "\",";

  result += "\"cid\":";
  result += "\"";
  result += String(ESP.getFlashChipId());
  result += "\",";
 
  result += "\"crs\":";
  result += "\"";
  result += String(ESP.getFlashChipRealSize());
  result += "\",";
 
  result += "\"csi\":";
  result += "\"";
  result += String(ESP.getFlashChipSize());
  result += "\",";
 
  result += "\"csp\":";
  result += "\"";
  result += String(ESP.getFlashChipSpeed());
  result += "\",";
 
  result += "\"cm\":";
  result += "\"";
  result += String(ESP.getFlashChipMode());
  result += "\"";

  result += "}";
  return result;
}

void sendDataToTarget(asyncHTTPrequest* pRequest, WiFiClient wifiClient, Settings * pSettings, WiFiSettings* pWifiSettings, String macAddress, bool withInfo)
{
  String targetServer = pSettings->getTargetServer();
  uint16_t port =  pSettings->getTargetPort();
  String path =  pSettings->getTargetPath();
  String url = targetServer + ":" + String(port) + path;
 
  // Note: BasicAuthentication does not allow any colon characters
  //       replace them with an underscore
  String key = macAddress;
  key.replace(":", "_");
  // Note: String end with \n character that has to be removed in the header
  String auth = "Basic " + base64::encode(key + ":" + pSettings->getDeviceKey());
  auth.replace("\n","");

  String postData = "{";
  postData += getSendData(pSettings, macAddress);
  if (withInfo == true) {
    postData += ",";
    postData += getSendInfo(pSettings, pWifiSettings, macAddress);
  }
  postData += "}";


  if (pRequest->readyState() == 0 || pRequest->readyState() == 4)
  {
    pRequest->open("POST", url.c_str());
    pRequest->setReqHeader("Content-Type", "application/json");
    pRequest->setReqHeader("Cache-Control", "no-cache");
    pRequest->setReqHeader("Connection", "keep-alive");
    pRequest->setReqHeader("Pragma", "no-cache");
    pRequest->setReqHeader("WWW-Authenticate", "Basic realm=\"model\", charset=\"UTF-8\"");
    pRequest->setReqHeader("Authorization", auth.c_str());
    pRequest->send(postData);
  }
}

String getAsyncResponse(asyncHTTPrequest* pRequest) {
  if (pRequest->readyState() == 4)
  {
    if (pRequest->responseHTTPcode() == 200)
    {
      return pRequest->responseText();;
    }
  }
  return "";
}
