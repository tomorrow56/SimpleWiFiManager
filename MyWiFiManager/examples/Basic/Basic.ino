#include <WiFi.h>
#include <MyWiFiManager.h>

MyWiFiManager wifiManager;

void setup() {
  Serial.begin(115200);
  
  // WebUIのタイトルを設定
  wifiManager.setWebUITitle("WiFiManager Basic");
  // テーマをライトモードに設定
  wifiManager.setWebUITheme(WM_WEBUI_THEME_LIGHT);
  // テーマをダークモードに設定
  // wifiManager.setWebUITheme(WM_WEBUI_THEME_DARK);

  // WiFi接続試行のタイムアウトを10秒に設定
  wifiManager.setConnectTimeout(10);
 
  // WiFi接続を自動で試行し、失敗した場合は設定ポータルを開始
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("Failed to connect and hit timeout");
    // リセットして再試行
    ESP.restart();
    delay(1000);
  }

  // WiFi接続成功
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // メインループ
}

