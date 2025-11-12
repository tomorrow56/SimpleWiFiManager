#ifndef SimpleWiFiManager_h
#define SimpleWiFiManager_h

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#else
#include <WiFi.h>
#include <WebServer.h>
#endif
#include <DNSServer.h>
#include <memory>

#if defined(ESP8266)
extern "C" {
  #include "user_interface.h"
}
#define ESP_getChipId()   (ESP.getChipId())
#else
#include <esp_wifi.h>
#define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())
#endif

#ifndef WIFI_MANAGER_MAX_PARAMS
#define WIFI_MANAGER_MAX_PARAMS 10
#endif

// WebUI Theme constants
#define WM_WEBUI_THEME_LIGHT 0
#define WM_WEBUI_THEME_DARK  1

// WiFiManagerParameter class
class WiFiManagerParameter {
  public:
    WiFiManagerParameter(const char *custom);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    const char *getID();
    const char *getValue();
    const char *getPlaceholder();
    int         getValueLength();
    const char *getCustomHTML();

  private:
    const char *_id;
    const char *_placeholder;
    const char *_customHTML;

    void init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);
    
  public:
    char       *_value;
    int        _length;
};

// Forward declaration for WebUI class
class WebUI;

class SimpleWiFiManager
{
  public:
    SimpleWiFiManager();
    ~SimpleWiFiManager();

    boolean       autoConnect();
    boolean       autoConnect(char const *apName, char const *apPassword = NULL);

    boolean       startConfigPortal();
    boolean       startConfigPortal(char const *apName, char const *apPassword = NULL);

    String        getConfigPortalSSID();
    String        getSSID();
    String        getPassword();
    void          resetSettings();

    void          setConnectTimeout(unsigned long seconds);
    void          setConfigPortalTimeout(unsigned long seconds);

    void          setDebugOutput(boolean debug);
    void          setMinimumSignalQuality(int quality = 8);
    void          setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    void          setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    void          setAPCallback( void (*func)(SimpleWiFiManager*) );
    void          setSaveConfigCallback( void (*func)(void) );
    void          addParameter(WiFiManagerParameter *p);
    void          setBreakAfterConfig(boolean shouldBreak);
    void          setCustomHeadElement(const char* element);
    void          setRemoveDuplicateAPs(boolean removeDuplicates);

    // テーマ関連の新しいメソッド
    void          setWebUITheme(int theme);
    int           getWebUITheme();
    void          setWebUITitle(const char* title);

  private:
    std::unique_ptr<DNSServer>        _dnsServer;
#ifdef ESP8266
    std::unique_ptr<ESP8266WebServer> _server;
#else
    std::unique_ptr<WebServer>        _server;
#endif

    // WebUI object
    WebUI* _webUI;

    void          setupConfigPortal();
    void          startWPS();
    
    // Handler methods
    void          handleRoot();
    void          handleWifi(boolean scan);
    void          handleWifiSave();
    void          handleInfo();
    void          handleReset();
    void          handleNotFound();
    void          handleThemeToggle();

    const char*   _apName                 = "no-net";
    const char*   _apPassword             = NULL;
    String        _ssid                   = "";
    String        _pass                   = "";
    unsigned long _configPortalTimeout    = 0;
    unsigned long _connectTimeout         = 60000;
    unsigned long _configPortalStart      = 0;

    IPAddress     _ap_static_ip;
    IPAddress     _ap_static_gw;
    IPAddress     _ap_static_sn;
    IPAddress     _sta_static_ip;
    IPAddress     _sta_static_gw;
    IPAddress     _sta_static_sn;

    int           _paramsCount            = 0;
    int           _minimumQuality         = -1;
    boolean       _removeDuplicateAPs     = true;
    boolean       _shouldBreakAfterConfig = false;
    boolean       _tryWPS                 = false;

    const char*   _customHeadElement      = "";
    const char*   _webUITitle             = "SimpleWiFiManager";

    int           status = WL_IDLE_STATUS;
    int           connectWifi(String ssid, String pass);
    uint8_t       waitForConnectResult();

    boolean       captivePortal();
    boolean       configPortalHasTimeout();

    const byte    DNS_PORT = 53;

    int           getRSSIasQuality(int RSSI);
    boolean       isIp(String str);
    String        toStringIp(IPAddress ip);

    boolean       connect;
    boolean       _debug = true;

    void (*_apcallback)(SimpleWiFiManager*) = NULL;
    void (*_savecallback)(void) = NULL;

    WiFiManagerParameter* _params[WIFI_MANAGER_MAX_PARAMS];

    template <typename Generic>
    void          DEBUG_WM(Generic text);

    template <class T>
    auto optionalIPFromString(T *obj, const char *s) -> decltype(  obj->fromString(s)  ) {
      return  obj->fromString(s);
    }
    auto optionalIPFromString(...) -> bool {
      DEBUG_WM("NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work.");
      return false;
    }
};

#endif

