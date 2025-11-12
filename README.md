# SimpleWiFiManager

A WiFiManager-like library with separated WebUI components for ESP8266/ESP32 microcontrollers.

## Overview

SimpleWiFiManager provides an easy way to configure WiFi credentials for ESP8266/ESP32 devices using a captive portal. The library features a modular design with separated WebUI components for better maintainability and customization.

## Features

- **Easy WiFi Configuration**: Set up WiFi credentials through a web interface
- **Captive Portal**: Automatic redirection to configuration page
- **Dual Platform Support**: Compatible with both ESP8266 and ESP32
- **Modular Design**: Separated WebUI components for better code organization
- **Theme Switching**: Light and Dark mode WebUI themes with toggle switch
- **Custom Parameters**: Add custom configuration parameters
- **Static IP Configuration**: Support for both AP and STA static IP settings
- **Timeout Management**: Configurable timeouts for connection and portal
- **Debug Output**: Built-in debugging capabilities
- **Signal Quality Filter**: Minimum signal quality threshold
- **Callback Support**: Custom callbacks for various events
- **M5Stack Support**: Dedicated examples for M5Stack devices with display integration
- **Advanced WiFi Reset**: Complete NVS partition clearing for thorough reset

## Installation

### Arduino IDE
1. Download the library as a ZIP file
2. Open Arduino IDE
3. Go to **Sketch** → **Include Library** → **Add .ZIP Library**
4. Select the downloaded ZIP file

### PlatformIO
Add to your `platformio.ini`:
```ini
lib_deps = 
    https://github.com/manus/SimpleWiFiManager.git
```

## Quick Start

### Basic Usage

```cpp
#include <MyWiFiManager.h>

MyWiFiManager wifiManager;

void setup() {
  Serial.begin(115200);
  
  // Try to connect with saved credentials or start config portal
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("Failed to connect and hit timeout");
    // Handle connection failure
  } else {
    Serial.println("WiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  // Your main code here
}
```

### Advanced Configuration

```cpp
#include <MyWiFiManager.h>

MyWiFiManager wifiManager;

void setup() {
  Serial.begin(115200);
  
  // Set custom AP name and password
  wifiManager.autoConnect("MyDevice-Config", "password123");
  
  // Configure timeouts
  wifiManager.setConfigPortalTimeout(180); // 3 minutes
  wifiManager.setConnectTimeout(30);       // 30 seconds
  
  // Set minimum signal quality (0-100%)
  wifiManager.setMinimumSignalQuality(20);
  
  // Enable debug output
  wifiManager.setDebugOutput(true);
  
  // Set static IP for AP mode
  IPAddress ap_ip(192, 168, 4, 1);
  IPAddress ap_gateway(192, 168, 4, 1);
  IPAddress ap_subnet(255, 255, 255, 0);
  wifiManager.setAPStaticIPConfig(ap_ip, ap_gateway, ap_subnet);
}
```

## API Reference

### Core Methods

#### `autoConnect()`
```cpp
boolean autoConnect();
boolean autoConnect(const char* apName, const char* apPassword = NULL);
```
Attempts to connect to saved WiFi credentials. If connection fails, starts configuration portal.

#### `startConfigPortal()`
```cpp
boolean startConfigPortal();
boolean startConfigPortal(const char* apName, const char* apPassword = NULL);
```
Forces the configuration portal to start immediately.

### Configuration Methods

#### `setConfigPortalTimeout()`
```cpp
void setConfigPortalTimeout(unsigned long seconds);
```
Sets timeout for the configuration portal (0 = no timeout).

#### `setConnectTimeout()`
```cpp
void setConnectTimeout(unsigned long seconds);
```
Sets timeout for WiFi connection attempts.

#### `setDebugOutput()`
```cpp
void setDebugOutput(boolean debug);
```
Enables or disables debug output to Serial.

#### `setMinimumSignalQuality()`
```cpp
void setMinimumSignalQuality(int quality = 8);
```
Sets minimum signal quality threshold (0-100%).

### Theme Configuration

#### `setWebUITheme()`
```cpp
void setWebUITheme(int theme);
```
Sets the WebUI theme. Available themes:
- `WM_WEBUI_THEME_LIGHT` (0) - Light theme
- `WM_WEBUI_THEME_DARK` (1) - Dark theme (default)

#### `getWebUITheme()`
```cpp
int getWebUITheme();
```
Returns the current WebUI theme setting.

#### `setWebUITitle()`
```cpp
void setWebUITitle(const char* title);
```
Sets a custom title for the WebUI interface.

### Network Configuration

#### `setAPStaticIPConfig()`
```cpp
void setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
```
Configures static IP settings for Access Point mode.

#### `setSTAStaticIPConfig()`
```cpp
void setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
```
Configures static IP settings for Station mode.

### Information Methods

#### `getSSID()` / `getPassword()`
```cpp
String getSSID();
String getPassword();
```
Returns the currently configured SSID and password.

#### `getConfigPortalSSID()`
```cpp
String getConfigPortalSSID();
```
Returns the SSID of the configuration portal.

### Utility Methods

#### `resetSettings()`
```cpp
void resetSettings();
```
Completely clears saved WiFi credentials from NVM (Non-Volatile Memory).

**Note**: This function performs the following operations:
- WiFi.disconnect(true) to disconnect and clear WiFi stack
- NVS partition erase and reinitialization
- Complete WiFi settings removal from ESP32's non-volatile memory

#### `addParameter()`
```cpp
void addParameter(WiFiManagerParameter *p);
```
Adds custom parameters to the configuration portal.

### Callback Methods

#### `setAPCallback()`
```cpp
void setAPCallback(void (*func)(MyWiFiManager*));
```
Sets callback function called when AP mode starts.

#### `setSaveConfigCallback()`
```cpp
void setSaveConfigCallback(void (*func)(void));
```
Sets callback function called when configuration is saved.

## Custom Parameters

Add custom configuration parameters to the web interface:

```cpp
#include <MyWiFiManager.h>

MyWiFiManager wifiManager;

// Custom parameter
WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", "mqtt.example.com", 40);
WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", "1883", 6);

void setup() {
  Serial.begin(115200);
  
  // Add custom parameters
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  
  // Set save config callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  if (!wifiManager.autoConnect("MyDevice")) {
    Serial.println("Failed to connect");
  }
}

void saveConfigCallback() {
  Serial.println("Configuration saved");
  Serial.print("MQTT Server: ");
  Serial.println(custom_mqtt_server.getValue());
  Serial.print("MQTT Port: ");
  Serial.println(custom_mqtt_port.getValue());
}
```

## WebUI Architecture

The library features a modular WebUI design:

- **MyWiFiManager**: Main class handling WiFi connection logic
- **WebUI**: Separated web interface handling HTTP requests and responses
- **WiFiManagerParameter**: Custom parameter management

This separation allows for:
- Better code organization
- Easier customization of web interface
- Improved maintainability
- Reduced memory footprint

## Supported Platforms

- **ESP8266**: All variants (NodeMCU, Wemos D1, etc.)
- **ESP32**: All variants (ESP32-WROOM, ESP32-S2, ESP32-C3, etc.)

## Dependencies

- **ESP8266**: ESP8266WiFi, ESP8266WebServer, DNSServer
- **ESP32**: WiFi, WebServer, DNSServer

## Configuration Portal Interface

When the configuration portal is active, users can:

1. **Scan Networks**: View available WiFi networks with signal strength
2. **Enter Credentials**: Input SSID and password
3. **Theme Toggle**: Switch between light and dark themes using the toggle switch
4. **Custom Parameters**: Configure additional parameters if added
5. **Device Information**: View device details and current settings
6. **Reset Settings**: Clear saved configurations

### Theme Features
- **Light/Dark Mode**: Toggle between light and dark themes
- **Persistent Settings**: Theme preference is saved in NVS memory
- **Responsive Design**: Optimized for both desktop and mobile devices
- **Compact Toggle**: Small, right-aligned theme switch for minimal UI impact

## Troubleshooting

### Common Issues

**Portal not accessible:**
- Check if device is in AP mode
- Connect to the AP network (default: "no-net")
- Navigate to 192.168.4.1

**Connection timeout:**
- Increase connection timeout: `setConnectTimeout(60)`
- Check WiFi credentials
- Verify network availability

**Memory issues:**
- Reduce custom parameters
- Disable debug output in production
- Use shorter strings for AP names

### Debug Output

Enable debug output to troubleshoot issues:
```cpp
wifiManager.setDebugOutput(true);
```

## License

This project is licensed under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Examples

The library includes several example sketches:

### Basic Example
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

### M5Stack Example
For M5Stack devices with display integration and button controls:
```cpp
#include <Arduino.h>
#include <M5Unified.h>
#include <MyWiFiManager.h>

MyWiFiManager wifiManager;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  
  Serial.begin(115200);
  
  // Display setup
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 10);
  M5.Display.println("MyWiFiManager");
  
  // Theme configuration
  wifiManager.setWebUITheme(WM_WEBUI_THEME_DARK);
  
  // WiFi connection with M5Stack integration
  if (!wifiManager.autoConnect("M5Stack-WiFi")) {
    M5.Display.println("Connection failed");
  } else {
    M5.Display.println("Connected!");
    M5.Display.println(WiFi.localIP());
  }
}
```

### WiFi Settings Clear Example
Complete WiFi settings reset utility:
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <nvs_flash.h>

void setup() {
  Serial.begin(115200);
  
  Serial.println("=== WiFi NVM Settings Clear Tool ===");
  
  // Method 1: Arduino WiFi library method
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  // Method 2: ESP-IDF NVS flash erase (more thorough)
  esp_err_t err = nvs_flash_erase_partition("nvs");
  if (err == ESP_OK) {
    Serial.println("NVS partition erased successfully");
  }
  
  // Method 3: Reinitialize NVS
  err = nvs_flash_init();
  if (err == ESP_OK) {
    Serial.println("NVS reinitialized successfully");
  }
  
  Serial.println("WiFi settings cleared. Restarting...");
  ESP.restart();
}
```

## Changelog

### Version 1.0.0
- Initial release
- Modular WebUI architecture
- ESP8266/ESP32 support
- Custom parameter support
- Captive portal functionality
- Light/Dark theme switching
- M5Stack integration examples
- Advanced NVS WiFi reset functionality
- Compact, right-aligned theme toggle switch
