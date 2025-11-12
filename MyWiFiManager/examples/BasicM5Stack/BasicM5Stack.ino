#include <Arduino.h>
#include <M5Unified.h>
#include <MyWiFiManager.h>

MyWiFiManager wifiManager;

// Button state variables
unsigned long buttonPressStart = 0;
bool buttonPressed = false;
bool wifiCleared = false;
const unsigned long LONG_PRESS_TIME = 3000; // 3 seconds

void setup() {
  // Initialize M5Unified
  auto cfg = M5.config();
  M5.begin(cfg);
  
  Serial.begin(115200);
  delay(100);

  // WebUIのタイトルを設定
  wifiManager.setWebUITitle("M5Stack Basic");
  // テーマをライトモードに設定
  //wifiManager.setWebUITheme(WM_WEBUI_THEME_LIGHT);
  // テーマをダークモードに設定
  wifiManager.setWebUITheme(WM_WEBUI_THEME_DARK);

  // Display initial message
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 10);
  M5.Display.println("MyWiFiManager");
  M5.Display.setCursor(10, 40);
  M5.Display.println("M5Stack Core");
  M5.Display.setTextSize(1);
  M5.Display.setCursor(10, 80);
  M5.Display.println("Hold A button 3sec");
  M5.Display.setCursor(10, 100);
  M5.Display.println("to clear WiFi settings");

  Serial.println("\nStarting MyWiFiManager for M5Stack Core (M5Unified)");
  Serial.println("Hold A button for 3 seconds to clear WiFi settings");

  // WiFi接続試行のタイムアウトを10秒に設定
  wifiManager.setConnectTimeout(10);

  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("Failed to connect and hit timeout");
    M5.Display.setCursor(10, 140);
    M5.Display.setTextColor(RED);
    M5.Display.println("WiFi connection failed");
    M5.Display.setCursor(10, 160);
    M5.Display.println("Connect to AutoConnectAP");
    delay(3000);
  } else {
    // WiFi connected
    Serial.println("WiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    // Display connection success
    M5.Display.setCursor(10, 140);
    M5.Display.setTextColor(GREEN);
    M5.Display.println("WiFi Connected!");
    M5.Display.setCursor(10, 160);
    M5.Display.print("IP: ");
    M5.Display.println(WiFi.localIP());
  }
}

void loop() {
  M5.update();
  
  // Check A button state
  if (M5.BtnA.isPressed() && !buttonPressed) {
    // Button just pressed
    buttonPressed = true;
    buttonPressStart = millis();
    
    // Visual feedback
    M5.Display.fillRect(10, 200, 300, 20, BLACK);
    M5.Display.setCursor(10, 200);
    M5.Display.setTextColor(YELLOW);
    M5.Display.println("Hold A button...");
    
    Serial.println("A button pressed - hold for 3 seconds to clear WiFi");
  }
  
  if (buttonPressed) {
    unsigned long pressDuration = millis() - buttonPressStart;
    
    // Show progress bar
    int progress = map(pressDuration, 0, LONG_PRESS_TIME, 0, 280);
    M5.Display.fillRect(10, 220, progress, 10, YELLOW);
    
    if (M5.BtnA.isPressed()) {
      // Still holding
      if (pressDuration >= LONG_PRESS_TIME && !wifiCleared) {
        // Long press detected - clear WiFi settings
        wifiCleared = true;
        
        Serial.println("Long press detected! Clearing WiFi settings...");
        
        // Visual feedback
        M5.Display.fillScreen(BLACK);
        M5.Display.setTextColor(RED);
        M5.Display.setTextSize(2);
        M5.Display.setCursor(10, 80);
        M5.Display.println("Clearing WiFi");
        M5.Display.setCursor(10, 110);
        M5.Display.println("Settings...");
        
        // Clear WiFi settings
        wifiManager.resetSettings();
        
        Serial.println("WiFi settings cleared! Restarting...");
        
        // Show completion message
        M5.Display.setTextColor(GREEN);
        M5.Display.setCursor(10, 150);
        M5.Display.println("Complete!");
        M5.Display.setCursor(10, 180);
        M5.Display.println("Restarting...");
        
        delay(2000);
        ESP.restart();
      }
    } else {
      // Button released before long press
      buttonPressed = false;
      wifiCleared = false;
      
      // Clear progress display
      M5.Display.fillRect(10, 200, 300, 40, BLACK);
      
      Serial.println("A button released");
    }
  }
  
  // Other M5Stack functionality can be added here
  delay(50);
}
