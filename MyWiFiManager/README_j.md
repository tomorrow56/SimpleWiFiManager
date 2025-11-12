# MyWiFiManager

ESP8266/ESP32マイコン用のWebUIコンポーネントを分離したWiFiManagerライクなライブラリです。

## 概要

MyWiFiManagerは、キャプティブポータルを使用してESP8266/ESP32デバイスのWiFi認証情報を簡単に設定する方法を提供します。このライブラリは、保守性とカスタマイズ性を向上させるために、WebUIコンポーネントを分離したモジュラー設計を特徴としています。

## 特徴

- **簡単なWiFi設定**: Webインターフェースを通じてWiFi認証情報を設定
- **キャプティブポータル**: 設定ページへの自動リダイレクト
- **デュアルプラットフォーム対応**: ESP8266とESP32の両方に対応
- **モジュラー設計**: より良いコード構成のためにWebUIコンポーネントを分離
- **テーマ切替**: ライト・ダークモードのWebUIテーマとトグルスイッチ
- **カスタムパラメータ**: カスタム設定パラメータの追加
- **静的IP設定**: APとSTA両方の静的IP設定をサポート
- **タイムアウト管理**: 接続とポータルの設定可能なタイムアウト
- **デバッグ出力**: 内蔵デバッグ機能
- **信号品質フィルタ**: 最小信号品質閾値
- **コールバックサポート**: 各種イベント用のカスタムコールバック
- **M5Stackサポート**: ディスプレイ統合されたM5Stackデバイス専用サンプル
- **高度なWiFiリセット**: 完全なNVSパーティションクリアによる徹底的なリセット

## インストール

### Arduino IDE
1. ライブラリをZIPファイルとしてダウンロード
2. Arduino IDEを開く
3. **スケッチ** → **ライブラリをインクルード** → **.ZIP形式のライブラリをインストール**
4. ダウンロードしたZIPファイルを選択

### PlatformIO
`platformio.ini`に追加:
```ini
lib_deps = 
    https://github.com/manus/MyWiFiManager.git
```

## クイックスタート

### 基本的な使用方法

```cpp
#include <MyWiFiManager.h>

MyWiFiManager wifiManager;

void setup() {
  Serial.begin(115200);
  
  // 保存された認証情報で接続を試行、失敗時は設定ポータルを開始
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("接続に失敗しタイムアウトしました");
    // 接続失敗の処理
  } else {
    Serial.println("WiFi接続成功！");
    Serial.print("IPアドレス: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  // メインコードをここに記述
}
```

### 高度な設定

```cpp
#include <MyWiFiManager.h>

MyWiFiManager wifiManager;

void setup() {
  Serial.begin(115200);
  
  // カスタムAP名とパスワードを設定
  wifiManager.autoConnect("MyDevice-Config", "password123");
  
  // タイムアウトを設定
  wifiManager.setConfigPortalTimeout(180); // 3分
  wifiManager.setConnectTimeout(30);       // 30秒
  
  // 最小信号品質を設定 (0-100%)
  wifiManager.setMinimumSignalQuality(20);
  
  // デバッグ出力を有効化
  wifiManager.setDebugOutput(true);
  
  // APモード用の静的IPを設定
  IPAddress ap_ip(192, 168, 4, 1);
  IPAddress ap_gateway(192, 168, 4, 1);
  IPAddress ap_subnet(255, 255, 255, 0);
  wifiManager.setAPStaticIPConfig(ap_ip, ap_gateway, ap_subnet);
}
```

## APIリファレンス

### コアメソッド

#### `autoConnect()`
```cpp
boolean autoConnect();
boolean autoConnect(const char* apName, const char* apPassword = NULL);
```
保存されたWiFi認証情報での接続を試行します。接続に失敗した場合、設定ポータルを開始します。

#### `startConfigPortal()`
```cpp
boolean startConfigPortal();
boolean startConfigPortal(const char* apName, const char* apPassword = NULL);
```
設定ポータルを強制的に即座に開始します。

### 設定メソッド

#### `setConfigPortalTimeout()`
```cpp
void setConfigPortalTimeout(unsigned long seconds);
```
設定ポータルのタイムアウトを設定します（0 = タイムアウトなし）。

#### `setConnectTimeout()`
```cpp
void setConnectTimeout(unsigned long seconds);
```
WiFi接続試行のタイムアウトを設定します。

#### `setDebugOutput()`
```cpp
void setDebugOutput(boolean debug);
```
シリアルへのデバッグ出力を有効または無効にします。

#### `setMinimumSignalQuality()`
```cpp
void setMinimumSignalQuality(int quality = 8);
```
最小信号品質閾値を設定します（0-100%）。

### テーマ設定

#### `setWebUITheme()`
```cpp
void setWebUITheme(int theme);
```
WebUIテーマを設定します。利用可能なテーマ:
- `WM_WEBUI_THEME_LIGHT` (0) - ライトテーマ
- `WM_WEBUI_THEME_DARK` (1) - ダークテーマ（デフォルト）

#### `getWebUITheme()`
```cpp
int getWebUITheme();
```
現在のWebUIテーマ設定を返します。

#### `setWebUITitle()`
```cpp
void setWebUITitle(const char* title);
```
WebUIインターフェースのカスタムタイトルを設定します。

### ネットワーク設定

#### `setAPStaticIPConfig()`
```cpp
void setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
```
アクセスポイントモード用の静的IP設定を構成します。

#### `setSTAStaticIPConfig()`
```cpp
void setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
```
ステーションモード用の静的IP設定を構成します。

### 情報取得メソッド

#### `getSSID()` / `getPassword()`
```cpp
String getSSID();
String getPassword();
```
現在設定されているSSIDとパスワードを返します。

#### `getConfigPortalSSID()`
```cpp
String getConfigPortalSSID();
```
設定ポータルのSSIDを返します。

### ユーティリティメソッド

#### `resetSettings()`
```cpp
void resetSettings();
```
NVM（不揮発性メモリ）から保存されたWiFi設定を完全にクリアします。

**注意**: この機能は以下の処理を実行します：
- WiFi.disconnect(true)によるWiFiスタックの切断とクリア
- NVSパーティションの消去と再初期化
- ESP32の不揮発性メモリからの完全なWiFi設定削除

#### `addParameter()`
```cpp
void addParameter(WiFiManagerParameter *p);
```
設定ポータルにカスタムパラメータを追加します。

### コールバックメソッド

#### `setAPCallback()`
```cpp
void setAPCallback(void (*func)(MyWiFiManager*));
```
APモード開始時に呼び出されるコールバック関数を設定します。

#### `setSaveConfigCallback()`
```cpp
void setSaveConfigCallback(void (*func)(void));
```
設定保存時に呼び出されるコールバック関数を設定します。

## カスタムパラメータ

Webインターフェースにカスタム設定パラメータを追加:

```cpp
#include <MyWiFiManager.h>

MyWiFiManager wifiManager;

// カスタムパラメータ
WiFiManagerParameter custom_mqtt_server("server", "MQTTサーバー", "mqtt.example.com", 40);
WiFiManagerParameter custom_mqtt_port("port", "MQTTポート", "1883", 6);

void setup() {
  Serial.begin(115200);
  
  // カスタムパラメータを追加
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  
  // 設定保存コールバックを設定
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  if (!wifiManager.autoConnect("MyDevice")) {
    Serial.println("接続に失敗しました");
  }
}

void saveConfigCallback() {
  Serial.println("設定が保存されました");
  Serial.print("MQTTサーバー: ");
  Serial.println(custom_mqtt_server.getValue());
  Serial.print("MQTTポート: ");
  Serial.println(custom_mqtt_port.getValue());
}
```

## WebUIアーキテクチャ

このライブラリはモジュラーWebUI設計を特徴としています:

- **MyWiFiManager**: WiFi接続ロジックを処理するメインクラス
- **WebUI**: HTTPリクエストとレスポンスを処理する分離されたWebインターフェース
- **WiFiManagerParameter**: カスタムパラメータ管理

この分離により以下が可能になります:
- より良いコード構成
- Webインターフェースのより簡単なカスタマイズ
- 保守性の向上
- メモリフットプリントの削減

## サポートプラットフォーム

- **ESP8266**: 全バリアント（NodeMCU、Wemos D1など）
- **ESP32**: 全バリアント（ESP32-WROOM、ESP32-S2、ESP32-C3など）

## 依存関係

- **ESP8266**: ESP8266WiFi、ESP8266WebServer、DNSServer
- **ESP32**: WiFi、WebServer、DNSServer

## 設定ポータルインターフェース

設定ポータルがアクティブな時、ユーザーは以下が可能です:

1. **ネットワークスキャン**: 利用可能なWiFiネットワークを信号強度と共に表示
2. **認証情報入力**: SSIDとパスワードを入力
3. **テーマ切替**: トグルスイッチを使用してライト・ダークテーマを切り替え
4. **カスタムパラメータ**: 追加された場合、追加パラメータを設定
5. **デバイス情報**: デバイス詳細と現在の設定を表示
6. **設定リセット**: 保存された設定をクリア

### テーマ機能
- **ライト/ダークモード**: ライト・ダークテーマ間の切り替え
- **永続設定**: テーマ設定はNVSメモリに保存
- **レスポンシブデザイン**: デスクトップ・モバイル両方に最適化
- **コンパクトトグル**: UIへの影響を最小限に抑える小型右寄せテーマスイッチ

## トラブルシューティング

### よくある問題

**ポータルにアクセスできない:**
- デバイスがAPモードになっているか確認
- APネットワークに接続（デフォルト: "no-net"）
- 192.168.4.1にアクセス

**接続タイムアウト:**
- 接続タイムアウトを増加: `setConnectTimeout(60)`
- WiFi認証情報を確認
- ネットワークの可用性を確認

**メモリ問題:**
- カスタムパラメータを削減
- 本番環境でデバッグ出力を無効化
- AP名により短い文字列を使用

### デバッグ出力

問題のトラブルシューティングのためにデバッグ出力を有効化:
```cpp
wifiManager.setDebugOutput(true);
```

## ライセンス

このプロジェクトはMITライセンスの下でライセンスされています。

## 貢献

貢献を歓迎します！バグや機能リクエストについては、プルリクエストの送信やイシューの開設をお気軽にどうぞ。

## サンプルコード

ライブラリには複数のサンプルスケッチが含まれています:

### 基本サンプル
```cpp
#include <WiFi.h>
#include <MyWiFiManager.h>

MyWiFiManager wifiManager;

void setup() {
  Serial.begin(115200);
  
  // WiFi接続を自動で試行し、失敗した場合は設定ポータルを開始
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("Failed to connect and hit timeout");
    ESP.restart();
    delay(1000);
  }

  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // メインループ
}
```

### M5Stackサンプル
ディスプレイ統合とボタン制御を備えたM5Stackデバイス用:
```cpp
#include <Arduino.h>
#include <M5Unified.h>
#include <MyWiFiManager.h>

MyWiFiManager wifiManager;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  
  Serial.begin(115200);
  
  // ディスプレイ設定
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 10);
  M5.Display.println("MyWiFiManager");
  
  // テーマ設定
  wifiManager.setWebUITheme(WM_WEBUI_THEME_DARK);
  
  // M5Stack統合でのWiFi接続
  if (!wifiManager.autoConnect("M5Stack-WiFi")) {
    M5.Display.println("Connection failed");
  } else {
    M5.Display.println("Connected!");
    M5.Display.println(WiFi.localIP());
  }
}
```

### WiFi設定クリアサンプル
完全なWiFi設定リセットユーティリティ:
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <nvs_flash.h>

void setup() {
  Serial.begin(115200);
  
  Serial.println("=== WiFi NVM Settings Clear Tool ===");
  
  // 方法1: Arduino WiFiライブラリメソッド
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  // 方法2: ESP-IDF NVSフラッシュ消去（より徹底的）
  esp_err_t err = nvs_flash_erase_partition("nvs");
  if (err == ESP_OK) {
    Serial.println("NVSパーティション消去成功");
  }
  
  // 方法3: NVS再初期化
  err = nvs_flash_init();
  if (err == ESP_OK) {
    Serial.println("NVS再初期化成功");
  }
  
  Serial.println("WiFi設定をクリアしました。再起動中...");
  ESP.restart();
}
```

## 変更履歴

### バージョン 1.0.0
- 初回リリース
- モジュラーWebUIアーキテクチャ
- ESP8266/ESP32サポート
- カスタムパラメータサポート
- キャプティブポータル機能
- ライト/ダークテーマ切替
- M5Stack統合サンプル
- 高度なNVS WiFiリセット機能
- コンパクト右寄せテーマトグルスイッチ
