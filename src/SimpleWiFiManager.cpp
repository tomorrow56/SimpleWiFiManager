#include "SimpleWiFiManager.h"
#include <nvs_flash.h>
#include "webui.h"

// WiFiManagerParameter implementation (same as original)
WiFiManagerParameter::WiFiManagerParameter(const char *custom) {
  _id = NULL;
  _placeholder = NULL;
  _length = 0;
  _value = NULL;

  _customHTML = custom;
}

WiFiManagerParameter::WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length) {
  init(id, placeholder, defaultValue, length, "");
}

WiFiManagerParameter::WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom) {
  init(id, placeholder, defaultValue, length, custom);
}

void WiFiManagerParameter::init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom) {
  _id = id;
  _placeholder = placeholder;
  _length = length;
  _value = new char[length + 1];
  for (int i = 0; i < length; i++) {
    _value[i] = 0;
  }
  if (defaultValue != NULL) {
    strncpy(_value, defaultValue, length);
  }

  _customHTML = custom;
}

const char* WiFiManagerParameter::getValue() {
  return _value;
}
const char* WiFiManagerParameter::getID() {
  return _id;
}
const char* WiFiManagerParameter::getPlaceholder() {
  return _placeholder;
}
int WiFiManagerParameter::getValueLength() {
  return _length;
}
const char* WiFiManagerParameter::getCustomHTML() {
  return _customHTML;
}

void WiFiManagerParameter::setValue(const char *defaultValue, int length) {
  snprintf(_value, length, defaultValue);
}

SimpleWiFiManager::SimpleWiFiManager() {
  _webUI = nullptr;
}

void SimpleWiFiManager::addParameter(WiFiManagerParameter *p) {
  _params[_paramsCount] = p;
  _paramsCount++;
  DEBUG_WM("Adding parameter");
  DEBUG_WM(p->getID());
}

boolean SimpleWiFiManager::autoConnect() {
  String ssid = "ESP" + String(ESP.getEfuseMac());
  return autoConnect(ssid.c_str(), NULL);
}

boolean SimpleWiFiManager::autoConnect(char const *apName, char const *apPassword) {
  DEBUG_WM(F(""));
  DEBUG_WM(F("AutoConnect"));

  if (WiFi.SSID() != "") {
    DEBUG_WM(F("Using last saved values, should be faster"));
    ETS_UART_INTR_DISABLE();
    wifi_station_disconnect();
    ETS_UART_INTR_ENABLE();

    if (connectWifi("", "") == WL_CONNECTED) {
      DEBUG_WM(F("IP Address:"));
      DEBUG_WM(WiFi.localIP());
      return true;
    }
  }

  return startConfigPortal(apName, apPassword);
}

boolean SimpleWiFiManager::configPortalHasTimeout(){
    if(_configPortalTimeout == 0 || wifi_softap_get_station_num() > 0){
        _configPortalStart = millis();
    }
    return (millis() > _configPortalStart + _configPortalTimeout);
}

boolean SimpleWiFiManager::startConfigPortal() {
  String ssid = "ESP" + String(ESP.getEfuseMac());
  return startConfigPortal(ssid.c_str(), NULL);
}

boolean SimpleWiFiManager::startConfigPortal(char const *apName, char const *apPassword) {
  if(!WiFi.mode(WIFI_AP_STA)) {
    DEBUG_WM(F("Could not set mode"));
    return false;
  }

  _apName = apName;
  _apPassword = apPassword;

  if (_ap_static_ip) {
    DEBUG_WM(F("Custom AP IP/GW/Subnet"));
    WiFi.softAPConfig(_ap_static_ip, _ap_static_gw, _ap_static_sn);
  }

  if (_apPassword != NULL) {
    if (strlen(_apPassword) < 8 || strlen(_apPassword) > 63) {
      DEBUG_WM(F("Invalid AccessPoint password. Ignoring"));
      _apPassword = NULL;
    }
    DEBUG_WM(_apPassword);
  }

  if (!WiFi.softAP(_apName, _apPassword)) {
    DEBUG_WM(F("Could not create softAP"));
    return false;
  }

  delay(500);
  DEBUG_WM(F("AP IP address: "));
  DEBUG_WM(WiFi.softAPIP());

  _server.reset(new WebServer(80));
  _dnsServer.reset(new DNSServer());
  _webUI = new WebUI(_server.get(), _dnsServer.get());

  _webUI->setupHandlers(
    std::bind(&SimpleWiFiManager::handleRoot, this),
    std::bind(&SimpleWiFiManager::handleWifi, this, std::placeholders::_1),
    std::bind(&SimpleWiFiManager::handleWifiSave, this),
    std::bind(&SimpleWiFiManager::handleInfo, this),
    std::bind(&SimpleWiFiManager::handleReset, this),
    std::bind(&SimpleWiFiManager::handleNotFound, this),
    std::bind(&SimpleWiFiManager::captivePortal, this),
    std::bind(&SimpleWiFiManager::handleThemeToggle, this)
  );

  _webUI->startDNSServer();

  if ( _apcallback != NULL) {
    _apcallback(this);
  }

  connect = false;
  _configPortalStart = millis();

  while(1) {
    _webUI->processDNSRequest();
    _webUI->handleClient();

    if (connect) {
      connect = false;
      delay(2000);
      DEBUG_WM(F("Connecting to new AP"));

      if (connectWifi(_ssid, _pass) != WL_CONNECTED) {
        DEBUG_WM(F("Failed to connect."));
      } else {
        WiFi.mode(WIFI_STA);
        DEBUG_WM(F("WiFi connected...yeey :)"));
        DEBUG_WM(F("IP Address:"));
        DEBUG_WM(WiFi.localIP());

        if ( _savecallback != NULL) {
          _savecallback();
        }
        break;
      }

      if (_shouldBreakAfterConfig) {
        break;
      }
    }
    yield();
  }

  _server.reset();
  _dnsServer.reset();
  return  WiFi.status() == WL_CONNECTED;
}

int SimpleWiFiManager::connectWifi(String ssid, String pass) {
  DEBUG_WM(F("Connecting as wifi client..."));

  if (ssid.length() > 0) {
    resetSettings();

    WiFi.mode(WIFI_STA);

    if (_sta_static_ip) {
      DEBUG_WM(F("Custom STA IP/GW/Subnet"));
      WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn);
      DEBUG_WM(WiFi.localIP());
    }
    WiFi.begin(ssid.c_str(), pass.c_str());
  } else {
    WiFi.begin();
  }

  int connRes = waitForConnectResult();
  DEBUG_WM ("Connection result: ");
  DEBUG_WM ( connRes );
  return connRes;
}

uint8_t SimpleWiFiManager::waitForConnectResult() {
  if (_connectTimeout == 0) {
    return WiFi.waitForConnectResult();
  } else {
    DEBUG_WM (F("Waiting for connection result with time out"));
    unsigned long start = millis();
    boolean keepConnecting = true;
    uint8_t status;
    while (keepConnecting) {
      status = WiFi.status();
      if (millis() > start + _connectTimeout) {
        keepConnecting = false;
        DEBUG_WM (F("Connection timed out"));
      }
      if (status == WL_CONNECTED || status == WL_CONNECT_FAILED) {
        keepConnecting = false;
      }
      delay(100);
    }
    return status;
  }
}

String SimpleWiFiManager::getConfigPortalSSID() {
  return _apName;
}

void SimpleWiFiManager::resetSettings() {
  DEBUG_WM(F("settings invalidated"));
  DEBUG_WM(F("THIS MAY CAUSE AP NOT TO START UP PROPERLY. YOU NEED TO COMMENT IT OUT AFTER ERASING NVS"));
  WiFi.disconnect(true);
  delay(200);
}

void SimpleWiFiManager::setConfigPortalTimeout(unsigned long seconds) {
  _configPortalTimeout = seconds * 1000;
}

void SimpleWiFiManager::setConnectTimeout(unsigned long seconds) {
  _connectTimeout = seconds * 1000;
}

void SimpleWiFiManager::setDebugOutput(boolean debug) {
  _debug = debug;
}

void SimpleWiFiManager::setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn) {
  _ap_static_ip = ip;
  _ap_static_gw = gw;
  _ap_static_sn = sn;
}

void SimpleWiFiManager::setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn) {
  _sta_static_ip = ip;
  _sta_static_gw = gw;
  _sta_static_sn = sn;
}

void SimpleWiFiManager::setMinimumSignalQuality(int quality) {
  _minimumQuality = quality;
}

void SimpleWiFiManager::setBreakAfterConfig(boolean shouldBreak) {
  _shouldBreakAfterConfig = shouldBreak;
}

void SimpleWiFiManager::setWebUITheme(int theme) {
  if (_webUI) {
    _webUI->setTheme(theme);
  }
}

int SimpleWiFiManager::getWebUITheme() {
  if (_webUI) {
    return _webUI->getTheme();
  }
  return WM_WEBUI_THEME_DARK; // デフォルト値
}

void SimpleWiFiManager::handleRoot() {
  DEBUG_WM(F("Handle root"));
  if (captivePortal()) {
    return;
  }

  String page = WebUI::HTTP_HEAD_START;
  page.replace("{v}", "Options");
  page += WebUI::HTTP_SCRIPT;
  page += _webUI->getCurrentStyle();
  page += _customHeadElement;
  page += WebUI::HTTP_HEAD_END;
  page += "<h1>";
  page += _apName;
  page += "</h1>";
  page += "<h3>" + String(_webUITitle) + "</h3>";
  
  // スライドスイッチを追加
  String themeToggle = WebUI::HTTP_THEME_TOGGLE;
  String checked = (_webUI->getTheme() == WM_WEBUI_THEME_DARK) ? "checked" : "";
  themeToggle.replace("{c}", checked);
  page += themeToggle;
  
  page += WebUI::HTTP_PORTAL_OPTIONS;
  page += WebUI::HTTP_END;

  _server->sendHeader("Content-Length", String(page.length()));
  _server->send(200, "text/html", page);
}

void SimpleWiFiManager::handleWifi(boolean scan) {
  String page = WebUI::HTTP_HEAD_START;
  page.replace("{v}", "Config ESP");
  page += WebUI::HTTP_SCRIPT;
  page += _webUI->getCurrentStyle();
  page += _customHeadElement;
  page += WebUI::HTTP_HEAD_END;

  if (scan) {
    int n = WiFi.scanNetworks();
    DEBUG_WM(F("Scan done"));
    if (n == 0) {
      DEBUG_WM(F("No networks found"));
      page += F("No networks found. Refresh to scan again.");
    } else {
      int indices[n];
      for (int i = 0; i < n; i++) {
        indices[i] = i;
      }

      for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
          if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            std::swap(indices[i], indices[j]);
          }
        }
      }

      if (_removeDuplicateAPs) {
        String cssid;
        for (int i = 0; i < n; i++) {
          if (indices[i] == -1) continue;
          cssid = WiFi.SSID(indices[i]);
          for (int j = i + 1; j < n; j++) {
            if (cssid == WiFi.SSID(indices[j])) {
              DEBUG_WM("DUP AP: " + WiFi.SSID(indices[j]));
              indices[j] = -1;
            }
          }
        }
      }

      for (int i = 0; i < n; i++) {
        if (indices[i] == -1) continue;
        DEBUG_WM(WiFi.SSID(indices[i]));
        DEBUG_WM(WiFi.RSSI(indices[i]));
        int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));

        if (_minimumQuality == -1 || _minimumQuality < quality) {
          String item = WebUI::HTTP_ITEM;
          String rssiQ;
          rssiQ += quality;
          item.replace("{v}", WiFi.SSID(indices[i]));
          item.replace("{r}", rssiQ);
#if defined(ESP8266)
          if (WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE) {
#else
          if (WiFi.encryptionType(indices[i]) != WIFI_AUTH_OPEN) {
#endif
            item.replace("{i}", "l");
          } else {
            item.replace("{i}", "");
          }
          page += item;
          delay(0);
        } else {
          DEBUG_WM(F("Skipping due to quality"));
        }
      }
      page += "<br/>";
    }
  }

  page += WebUI::HTTP_FORM_START;
  char parLength[2];
  for (int i = 0; i < _paramsCount; i++) {
    if (_params[i] == NULL) {
      break;
    }

    String pitem = WebUI::HTTP_FORM_PARAM;
    if (_params[i]->getID() != NULL) {
      pitem.replace("{i}", _params[i]->getID());
      pitem.replace("{n}", _params[i]->getID());
      pitem.replace("{p}", _params[i]->getPlaceholder());
      snprintf(parLength, 2, "%d", _params[i]->getValueLength());
      pitem.replace("{l}", parLength);
      pitem.replace("{v}", _params[i]->getValue());
      pitem.replace("{c}", _params[i]->getCustomHTML());
    } else {
      pitem = _params[i]->getCustomHTML();
    }

    page += pitem;
  }
  if (_params[0] != NULL) {
    page += "<br/>";
  }

  page += WebUI::HTTP_FORM_END;

  page += WebUI::HTTP_SCAN_LINK;

  page += WebUI::HTTP_END;

  _server->sendHeader("Content-Length", String(page.length()));
  _server->send(200, "text/html", page);
}

void SimpleWiFiManager::handleWifiSave() {
  DEBUG_WM(F("WiFi save"));

  for (int i = 0; i < _paramsCount; i++) {
    if (_params[i] == NULL) {
      break;
    }
    String value = _server->arg(_params[i]->getID()).c_str();
    value.toCharArray(_params[i]->_value, _params[i]->_length);
    DEBUG_WM(F("Parameter"));
    DEBUG_WM(_params[i]->getID());
    DEBUG_WM(value);
  }

  if (_server->arg("s") != "") {
    _ssid = _server->arg("s");
    _pass = _server->arg("p");

    String page = WebUI::HTTP_HEAD_START;
    page.replace("{v}", "Credentials Saved");
    page += WebUI::HTTP_SCRIPT;
    page += _webUI->getCurrentStyle();
    page += _customHeadElement;
    page += WebUI::HTTP_HEAD_END;
    page += WebUI::HTTP_SAVED;
    page += WebUI::HTTP_END;
    _server->sendHeader("Content-Length", String(page.length()));
    _server->send(200, "text/html", page);

    DEBUG_WM(F("Sent wifi save page"));

    connect = true;
  } else {
    DEBUG_WM(F("Sent wifi save page"));
  }
}

void SimpleWiFiManager::handleInfo() {
  DEBUG_WM(F("Info"));

  String page = WebUI::HTTP_HEAD_START;
  page.replace("{v}", "Info");
  page += WebUI::HTTP_SCRIPT;
  page += _webUI->getCurrentStyle();
  page += _customHeadElement;
  page += WebUI::HTTP_HEAD_END;
  page += F("Chip ID: ");
  page += ESP.getEfuseMac();
  page += F("<br/>Flash Chip ID: ");
  page += ESP.getFlashChipSize();
  page += F("<br/>IDE Flash Size: ");
  page += ESP.getFlashChipSize();
  page += F("<br/>Real Flash Size: ");
  page += ESP.getFlashChipSize();
  page += F("<br/>Soft AP IP: ");
  page += WiFi.softAPIP().toString();
  page += F("<br/>Soft AP MAC: ");
  page += WiFi.softAPmacAddress();
  page += F("<br/>Station MAC: ");
  page += WiFi.macAddress();
  page += F("<br/>");
  page += WebUI::HTTP_END;

  _server->sendHeader("Content-Length", String(page.length()));
  _server->send(200, "text/html", page);

  DEBUG_WM(F("Sent info page"));
}

void SimpleWiFiManager::handleReset() {
  DEBUG_WM(F("Reset"));

  String page = WebUI::HTTP_HEAD_START;
  page.replace("{v}", "Info");
  page += WebUI::HTTP_SCRIPT;
  page += _webUI->getCurrentStyle();
  page += _customHeadElement;
  page += WebUI::HTTP_HEAD_END;
  page += F("Module will reset in a few seconds.");
  page += WebUI::HTTP_END;
  _server->sendHeader("Content-Length", String(page.length()));
  _server->send(200, "text/html", page);

  DEBUG_WM(F("Sent reset page"));
  delay(5000);
  ESP.restart();
  delay(2000);
}

void SimpleWiFiManager::handleNotFound() {
  if (captivePortal()) {
    return;
  }
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += _server->uri();
  message += "\nMethod: ";
  message += ( _server->method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += _server->args();
  message += "\n";

  for ( uint8_t i = 0; i < _server->args(); i++ ) {
    message += " " + _server->argName ( i ) + ": " + _server->arg ( i ) + "\n";
  }
  _server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  _server->sendHeader("Pragma", "no-cache");
  _server->sendHeader("Expires", "-1");
  _server->sendHeader("Content-Length", String(message.length()));
  _server->send(404, "text/plain", message);
}

void SimpleWiFiManager::handleThemeToggle() {
  DEBUG_WM(F("Theme toggle"));
  
  int currentTheme = _webUI->getTheme();
  int newTheme = (currentTheme == WM_WEBUI_THEME_LIGHT) ? WM_WEBUI_THEME_DARK : WM_WEBUI_THEME_LIGHT;
  _webUI->setTheme(newTheme);
  
  _server->send(200, "text/plain", "Theme toggled");
}

boolean SimpleWiFiManager::captivePortal() {
  if (!isIp(_server->hostHeader()) ) {
    DEBUG_WM(F("Request redirected to captive portal"));
    _server->sendHeader("Location", String("http://") + toStringIp(_server->client().localIP()), true);
    _server->send ( 302, "text/plain", "");
    _server->client().stop();
    return true;
  }
  return false;
}

template <typename Generic>
void SimpleWiFiManager::DEBUG_WM(Generic text) {
  if (_debug) {
    Serial.print("*WM: ");
    Serial.println(text);
  }
}

int SimpleWiFiManager::getRSSIasQuality(int RSSI) {
  int quality = 0;

  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}

boolean SimpleWiFiManager::isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

String SimpleWiFiManager::toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

void SimpleWiFiManager::setAPCallback(void (*func)(SimpleWiFiManager*)) {
  _apcallback = func;
}

void SimpleWiFiManager::setSaveConfigCallback(void (*func)(void)) {
  _savecallback = func;
}

void SimpleWiFiManager::setCustomHeadElement(const char* element) {
  _customHeadElement = element;
}

void SimpleWiFiManager::setRemoveDuplicateAPs(boolean removeDuplicates) {
  _removeDuplicateAPs = removeDuplicates;
}

String SimpleWiFiManager::getSSID() {
  if (_ssid == "") {
    DEBUG_WM(F("Reading SSID"));
    _ssid = WiFi.SSID();
    DEBUG_WM(F("SSID: "));
    DEBUG_WM(_ssid);
  }
  return _ssid;
}

String SimpleWiFiManager::getPassword() {
  if (_pass == "") {
    DEBUG_WM(F("Reading Password"));
    _pass = WiFi.psk();
    DEBUG_WM(F("Password: "));
    DEBUG_WM(_pass);
  }
  return _pass;
}

void SimpleWiFiManager::setWebUITitle(const char* title) {
  _webUITitle = title;
}

