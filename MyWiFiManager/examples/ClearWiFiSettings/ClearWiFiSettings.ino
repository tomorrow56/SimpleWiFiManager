#include <Arduino.h>
#include <WiFi.h>
#include <nvs_flash.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== WiFi NVM Settings Clear Tool ===");
  
  // Method 1: Arduino WiFi library method
  Serial.println("1. Clearing WiFi settings via WiFi.disconnect(true)...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(500);
  
  // Method 2: ESP-IDF NVS flash erase (more thorough)
  Serial.println("2. Erasing NVS WiFi partition...");
  esp_err_t err = nvs_flash_erase_partition("nvs");
  if (err == ESP_OK) {
    Serial.println("   NVS partition erased successfully");
  } else {
    Serial.printf("   NVS erase failed: %s\n", esp_err_to_name(err));
  }
  
  // Method 3: Initialize NVS again
  Serial.println("3. Reinitializing NVS...");
  err = nvs_flash_init();
  if (err == ESP_OK) {
    Serial.println("   NVS reinitialized successfully");
  } else {
    Serial.printf("   NVS init failed: %s\n", esp_err_to_name(err));
  }
  
  Serial.println("\n=== WiFi Settings Cleared ===");
  Serial.println("All stored WiFi credentials have been removed from NVM.");
  Serial.println("You can now upload your main sketch.");
  Serial.println("Device will restart in 5 seconds...");
  
  delay(5000);
  ESP.restart();
}

void loop() {
  // Nothing to do here
}
