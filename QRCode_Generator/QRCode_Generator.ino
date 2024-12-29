#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <qrcode_st7789.h>
#include <BluetoothSerial.h>

// Pin Definitions for LilyGo TTGO T-Display
#define TFT_RST   23    
#define TFT_CS    5     
#define TFT_DC    16  
#define TFT_BACKLIGHT 4
#define TFT_MOSI 19
#define TFT_SCLK 18  

// TFT Display and QR Code Initialization
Adafruit_ST7789 display = Adafruit_ST7789(&SPI, TFT_CS, TFT_DC, TFT_RST);
QRcode_ST7789 qrcode(&display);

// Bluetooth Initialization
BluetoothSerial SerialBT;

// Buffer for incoming Bluetooth messages
String bluetoothInput = "";

// Function to display QR Code dynamically
void displayQRCode(const String &data) {
    display.fillScreen(ST77XX_WHITE);
    qrcode.init();
    qrcode.create(data.c_str());
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");

    // Initialize TFT Display
    SPI.begin(TFT_SCLK, TFT_RST, TFT_MOSI, TFT_CS);
    display.init(134, 238);  // Adjust for your screen resolution
    pinMode(TFT_BACKLIGHT, OUTPUT);
    digitalWrite(TFT_BACKLIGHT, HIGH);

    // Initialize QR Code
    qrcode.init();
    display.fillScreen(ST77XX_WHITE);

    // Start Bluetooth
    SerialBT.begin("TTGO_QR_Display");  // Bluetooth Device Name
    Serial.println("Bluetooth started! Pair with 'TTGO_QR_Display'");
    
    // Display initial QR Code
    displayQRCode("Waiting for Bluetooth Input...");
}

void loop() {
    // Check if Bluetooth has data
    if (SerialBT.available()) {
        char receivedChar = SerialBT.read();

        // If Enter key is pressed, generate QR code
        if (receivedChar == '\n' || receivedChar == '\r') {
            if (bluetoothInput.length() > 0) {
                Serial.println("Bluetooth Input Received: " + bluetoothInput);
                displayQRCode(bluetoothInput);
                bluetoothInput = ""; // Clear the buffer
            }
        } else {
            // Append the character to the buffer
            bluetoothInput += receivedChar;
        }
    }
}
