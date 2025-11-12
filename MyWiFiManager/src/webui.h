#ifndef WebUI_h
#define WebUI_h

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif
#include <DNSServer.h>
#include <functional>

#define WM_WEBUI_THEME_LIGHT 0
#define WM_WEBUI_THEME_DARK 1

class WebUI {
public:
    WebUI(WebServer* server, DNSServer* dnsServer);

    void setupHandlers(std::function<void(void)> handleRootCb, 
                       std::function<void(bool)> handleWifiCb, 
                       std::function<void(void)> handleWifiSaveCb, 
                       std::function<void(void)> handleInfoCb, 
                       std::function<void(void)> handleResetCb, 
                       std::function<void(void)> handleNotFoundCb, 
                       std::function<bool(void)> captivePortalCb,
                       std::function<void(void)> handleThemeToggleCb);

    void startDNSServer();
    void processDNSRequest();
    void handleClient();

    // テーマ設定
    void setTheme(int theme);
    int getTheme();

    // HTML content strings
    static const char HTTP_HEAD_START[];

    static const char HTTP_SCRIPT[];
    static const char HTTP_HEAD_END[];
    static const char HTTP_PORTAL_OPTIONS[];
    static const char HTTP_THEME_TOGGLE[];
    static const char HTTP_ITEM[];
    static const char HTTP_FORM_START[];
    static const char HTTP_FORM_PARAM[];
    static const char HTTP_FORM_END[];
    static const char HTTP_SCAN_LINK[];
    static const char HTTP_SAVED[];
    static const char HTTP_END[];

    // 現在のテーマに応じたスタイルを取得
    const char* getCurrentStyle();

    // Color definitions
    static const char LIGHT_BACKGROUND_COLOR[];
    static const char LIGHT_TEXT_COLOR[];
    static const char LIGHT_BUTTON_COLOR[];
    static const char LIGHT_BUTTON_TEXT_COLOR[];
    static const char DARK_BACKGROUND_COLOR[];
    static const char DARK_TEXT_COLOR[];
    static const char DARK_BUTTON_COLOR[];
    static const char DARK_BUTTON_TEXT_COLOR[];

    // Functions to get current colors based on theme
    const char* getCurrentBackgroundColor();
    const char* getCurrentTextColor();
    const char* getCurrentButtonColor();
    const char* getCurrentButtonTextColor();

private:
    WebServer* _server;
    DNSServer* _dnsServer;
    int _currentTheme;
};

#endif

