#include "webui.h"
#include <WiFi.h>
#include <Preferences.h>

// HTML content strings
const char WebUI::LIGHT_BACKGROUND_COLOR[] PROGMEM = "#FFFFFF";
const char WebUI::LIGHT_TEXT_COLOR[] PROGMEM = "#333333";
const char WebUI::LIGHT_BUTTON_COLOR[] PROGMEM = "#1fa3ec";
const char WebUI::LIGHT_BUTTON_TEXT_COLOR[] PROGMEM = "#fff";

const char WebUI::DARK_BACKGROUND_COLOR[] PROGMEM = "#2c3e50";
const char WebUI::DARK_TEXT_COLOR[] PROGMEM = "#ecf0f1";
const char WebUI::DARK_BUTTON_COLOR[] PROGMEM = "#3498db";
const char WebUI::DARK_BUTTON_TEXT_COLOR[] PROGMEM = "#fff";

// HTML content strings
const char WebUI::HTTP_HEAD_START[] PROGMEM      = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";



const char WebUI::HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();} function toggleTheme(){fetch('/theme-toggle',{method:'POST'}).then(()=>{location.reload();});}</script>";
const char WebUI::HTTP_HEAD_END[] PROGMEM        = "</head><body><div style=\'text-align:center;display:inline-block;min-width:260px;\'>";
const char WebUI::HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/>";

const char WebUI::HTTP_THEME_TOGGLE[] PROGMEM    = "<div class=\"theme-toggle\"><span class=\"theme-label\">Light</span><label class=\"switch\"><input type=\"checkbox\" {c} onchange=\"toggleTheme()\"><span class=\"slider\"></span></label><span class=\"theme-label\">Dark</span></div>";

const char WebUI::HTTP_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";

const char WebUI::HTTP_FORM_START[] PROGMEM      = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='password'><br/>";

const char WebUI::HTTP_FORM_PARAM[] PROGMEM      = "<br/><input id='{i}' name='{n}' length={l} placeholder='{p}' value='{v}' {c}>";

const char WebUI::HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>save</button></form>";

const char WebUI::HTTP_SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";

const char WebUI::HTTP_SAVED[] PROGMEM           = "<div>Your Wi-Fi connection information has been saved.<br />This device will connect to the selected SSID.<br />If the connection fails, reboot and try again.</div>";

const char WebUI::HTTP_END[] PROGMEM             = "</div></body></html>";

WebUI::WebUI(WebServer* server, DNSServer* dnsServer) : _server(server), _dnsServer(dnsServer) {
    // Preferencesからテーマ設定を読み込み
    Preferences preferences;
    preferences.begin("webui", false);
    _currentTheme = preferences.getInt("theme", WM_WEBUI_THEME_DARK); // デフォルトはダークモード
    preferences.end();
}

void WebUI::setupHandlers(std::function<void(void)> handleRootCb, 
                           std::function<void(bool)> handleWifiCb, 
                           std::function<void(void)> handleWifiSaveCb, 
                           std::function<void(void)> handleInfoCb, 
                           std::function<void(void)> handleResetCb, 
                           std::function<void(void)> handleNotFoundCb, 
                           std::function<bool(void)> captivePortalCb,
                           std::function<void(void)> handleThemeToggleCb) {
    _server->on("/", handleRootCb);
    _server->on("/wifi", std::bind(handleWifiCb, true));
    _server->on("/0wifi", std::bind(handleWifiCb, false));
    _server->on("/wifisave", handleWifiSaveCb);
    _server->on("/i", handleInfoCb);
    _server->on("/r", handleResetCb);
    _server->on("/theme-toggle", HTTP_POST, handleThemeToggleCb);
    _server->onNotFound(handleNotFoundCb);
    _server->begin();
}

void WebUI::startDNSServer() {
    _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    _dnsServer->start(53, "*", WiFi.softAPIP());
}

void WebUI::processDNSRequest() {
    _dnsServer->processNextRequest();
}

void WebUI::handleClient() {
    _server->handleClient();
}

void WebUI::setTheme(int theme) {
    _currentTheme = theme;
    
    // Preferencesにテーマ設定を保存
    Preferences preferences;
    preferences.begin("webui", false);
    preferences.putInt("theme", _currentTheme);
    preferences.end();
}

int WebUI::getTheme() {
    return _currentTheme;
}

const char* WebUI::getCurrentStyle() {
    String style = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;background-color:" + String(getCurrentBackgroundColor()) + ";color:" + String(getCurrentTextColor()) + ";} button{border:0;border-radius:0.3rem;background-color:" + String(getCurrentButtonColor()) + ";color:" + String(getCurrentButtonTextColor()) + ";line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9I2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;} a{color:" + String(getCurrentTextColor()) + ";text-decoration:none;} a:hover{color:" + String(getCurrentButtonColor()) + ";} .theme-toggle{margin:10px 0;display:flex;align-items:center;justify-content:flex-end;gap:8px;} .theme-label{font-size:0.8rem;color:" + String(getCurrentTextColor()) + ";} .switch{position:relative;display:inline-block;width:40px;height:22px;} .switch input{opacity:0;width:0;height:0;} .slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:" + String((_currentTheme == WM_WEBUI_THEME_LIGHT) ? "#ccc" : "#555") + ";transition:.4s;border-radius:22px;} .slider:before{position:absolute;content:\"\";height:18px;width:18px;left:2px;bottom:2px;background-color:white;transition:.4s;border-radius:50%;} input:checked + .slider{background-color:" + String((_currentTheme == WM_WEBUI_THEME_LIGHT) ? "#28a745" : "#e74c3c") + ";} input:checked + .slider:before{transform:translateX(18px);}</style>";
    static char styleBuffer[2048]; // Adjust size as needed
    style.toCharArray(styleBuffer, sizeof(styleBuffer));
    return styleBuffer;
}



const char* WebUI::getCurrentBackgroundColor() {
    return (_currentTheme == WM_WEBUI_THEME_LIGHT) ? LIGHT_BACKGROUND_COLOR : DARK_BACKGROUND_COLOR;
}

const char* WebUI::getCurrentTextColor() {
    return (_currentTheme == WM_WEBUI_THEME_LIGHT) ? LIGHT_TEXT_COLOR : DARK_TEXT_COLOR;
}

const char* WebUI::getCurrentButtonColor() {
    return (_currentTheme == WM_WEBUI_THEME_LIGHT) ? LIGHT_BUTTON_COLOR : DARK_BUTTON_COLOR;
}

const char* WebUI::getCurrentButtonTextColor() {
    return (_currentTheme == WM_WEBUI_THEME_LIGHT) ? LIGHT_BUTTON_TEXT_COLOR : DARK_BUTTON_TEXT_COLOR;
}


