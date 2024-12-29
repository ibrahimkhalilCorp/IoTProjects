#include <PNGdec.h>
#include "panda.h" // Image is stored here in an 8-bit array
#include <DHT.h>
#include <TFT_eSPI.h>
#include <BluetoothSerial.h>

#define MAX_IMAGE_WIDTH 240 // Adjust for your images

PNG png; // PNG decoder instance

// Pins and constants for sensors
#define MQ_PIN 12        // MQ Sensor connected to pin 12 (analog)
#define RELAY_PIN 27     // Relay control connected to pin 27
#define DHT_PIN 17       // DHT11 data pin connected to pin 17
#define DHT_TYPE DHT11   // DHT sensor type
#define BATTERY_VOLTAGE_PIN 34 // ADC pin for battery voltage
#define USB_PIN 2         // Pin for USB detection

DHT dht(DHT_PIN, DHT_TYPE);
BluetoothSerial SerialBT;

// TFT display object
TFT_eSPI tft = TFT_eSPI();

int gasThreshold = 750;      // Minimum gas concentration threshold for testing
int tempThresholdHigh = 27;  // High temperature threshold for testing
int tempThresholdLow = 5;    // Low temperature threshold for testing

// Image display coordinates
int16_t xpos = 0;
int16_t ypos = 0;

unsigned long lastBatteryDisplayTime = 0;
const unsigned long batteryDisplayInterval = 15000; // 15 seconds
const unsigned long batteryDisplayDuration = 2000; // 2 seconds

void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

void showQRCode() {
  tft.fillScreen(TFT_BLACK); // Clear any previous content
  int16_t rc = png.openFLASH((uint8_t *)panda, sizeof(panda), pngDraw);
  if (rc == PNG_SUCCESS) {
    Serial.println("Successfully opened PNG file");
    tft.startWrite();
    png.decode(NULL, 0);
    tft.endWrite();
  } else {
    Serial.println("Failed to open PNG file");
  }
  delay(10000); // Display QR code for 10 seconds
}

void showBatteryHealth() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(0, 0);
  tft.println("Battery Info:");

  bool usbConnected = !digitalRead(USB_PIN); // LOW means connected
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.printf("Charging: %s\n", usbConnected ? "Yes" : "No");

  int rawVoltage = analogRead(BATTERY_VOLTAGE_PIN);
  float voltage = rawVoltage * (3.7 / 4095.0); // Adjust scaling
  tft.printf("Voltage: %.2fV\n", voltage);

  tft.printf("Current: %.2fA\n", 0.5); // Placeholder for current sensor
  delay(batteryDisplayDuration);
  tft.fillScreen(TFT_BLACK);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("AKIJ_Poultry"); // Initialize Bluetooth
  Serial.println("Using the PNGdec library");

  // Initialize the TFT display
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // Initialize sensors
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(USB_PIN, INPUT_PULLUP);
  dht.begin();

  // Show Welcome Message
  tft.setTextSize(2);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setCursor(20, 30);
  tft.println("Welcome to");
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(20, 60);
  tft.println("AKIJ Agro Tech");
  delay(3000);

  // Show QR code first
  showQRCode();
}

void loop() {
  if (millis() - lastBatteryDisplayTime > batteryDisplayInterval) {
    lastBatteryDisplayTime = millis();
    showBatteryHealth();
  }

  int gasValue = analogRead(MQ_PIN);
  float temperature = dht.readTemperature();

  if (isnan(temperature)) {
  Serial.println("Failed to read temperature from DHT sensor!");
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println("Failed to read temp!");
  SerialBT.println("Failed to read temp!");
} else {
  Serial.print("Temperature: ");
  Serial.println(temperature);

  // Create JSON string
  char jsonString[256];
  snprintf(jsonString, sizeof(jsonString),
           "[{\"Temp\": %.2f, \"icon\": \"coolant-temperature\"}, {\"Gas\": %d, \"icon\": \"run\"}]",
           temperature, gasValue);

  SerialBT.println(jsonString);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.printf("Temp: %.2f C\n", temperature);

  tft.setCursor(0, 40);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.printf("Gas Value: %d\n", gasValue);

  if (gasValue > gasThreshold || temperature > tempThresholdHigh) {
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Gas level or Temperature high, turning on devices");
    tft.setCursor(0, 80);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("Devices ON");
  } else if (gasValue <= gasThreshold && temperature <= tempThresholdHigh) {
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Gas level and Temperature normal, turning off devices");
    tft.setCursor(0, 80);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Devices OFF");
  }

  if (temperature < tempThresholdLow) {
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Temperature low, turning off devices");
    tft.setCursor(0, 120);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.println("Low Temp, OFF");
  }
}

  delay(2000);
}
