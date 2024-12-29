#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <MPU6050_light.h>
#include <TFT_eSPI.h> // Include the graphics library
#include <PNGdec.h>
#include <BluetoothSerial.h>
#include "panda.h" // Image is stored here in an 8-bit array

// Initialize Sensors
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
MPU6050 mpu(Wire);
BluetoothSerial SerialBT;

// Initialize TFT Display
TFT_eSPI tft = TFT_eSPI();
PNG png;

#define MAX_IMAGE_WIDTH 240
int16_t xpos = 0;
int16_t ypos = 0;

// Battery and USB setup
#define USB_PIN 2
#define BATTERY_VOLTAGE_PIN 34

void setup() {
  // Serial for Debugging
  Serial.begin(115200);
  SerialBT.begin("AKIJ_Cattle");
  Serial.println("T-Display with MLX90614, MPU6050, and PNG Image Display");

  // Initialize pins
  pinMode(USB_PIN, INPUT_PULLUP);

  // Initialize I2C for both sensors
  Wire.begin(21, 22);

  // Initialize MLX90614
  if (!initializeMLX90614()) {
    Serial.println("MLX90614 initialization failed.");
    while (1); // Halt execution
  }

  // Initialize MPU6050
  if (!initializeMPU6050()) {
    Serial.println("MPU6050 initialization failed.");
    while (1); // Halt execution
  }

  // Initialize TFT Display
  if (!initializeTFT()) {
    Serial.println("TFT initialization failed.");
    while (1); // Halt execution
  }

  // Show Welcome Message
  tft.setTextSize(2.5);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setCursor(20, 30);
  tft.println("Welcome to");
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(20, 60);
  tft.println("AKIJ Agro Tech");
  delay(3000);

  // Show PNG image for 10 seconds
  tft.fillScreen(TFT_BLACK);
  int16_t rc = png.openFLASH((uint8_t *)panda, sizeof(panda), pngDraw);
  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    png.decode(NULL, 0);
    tft.endWrite();
  }
  delay(10000);
}


void loop() {
  // Read temperature from MLX90614
  float ambientTemp = mlx.readAmbientTempC();
  float objectTemp = mlx.readObjectTempC();

  // Display MLX90614 data
  if (ambientTemp != -127.0 && objectTemp != -127.0) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_RED, TFT_BLACK); // Red for Ambient Temp
    tft.setCursor(0, 10);
    tft.printf("Ambient Temp:\n\n%.2f C", ambientTemp);
    delay(3000);

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK); // Orange for Object Temp
    tft.setCursor(0, 10);
    tft.printf("Body Temp:\n\n%.2f C", objectTemp);
    delay(3000);
  } else {
    Serial.println("Failed to read temperature from MLX90614.");
  }

  // Update MPU6050 data
  mpu.update();

  // Read accelerometer data
  float accelX = mpu.getAccX();
  float accelY = mpu.getAccY();
  float accelZ = mpu.getAccZ();

  // Display Accelerometer Data
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK); // Green for Accelerometer
  tft.setCursor(0, 10);
  tft.printf("Accel: \n\nAccel X: %.2f\nAccel Y: %.2f\nAccel Z: %.2f", accelX, accelY, accelZ);
  delay(3000);

  // Read gyroscope data
  float gyroX = mpu.getGyroX();
  float gyroY = mpu.getGyroY();
  float gyroZ = mpu.getGyroZ();

  // Display Gyroscope Data
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK); // Cyan for Gyroscope
  tft.setCursor(0, 10);
  tft.printf("Gyro: \n\nGyro X: %.2f\nGyro Y: %.2f\nGyro Z: %.2f", gyroX, gyroY, gyroZ);
  delay(3000);

  // Create JSON string for Bluetooth transmission
  char jsonString[512];
  snprintf(jsonString, sizeof(jsonString),
           "[{\"AmbientTemp\": %.2f, \"icon\": \"coolant-temperature\"}, "
           "{\"BodyTemp\": %.2f, \"icon\": \"coolant-temperature\"}, "
           "{\"Gyro_X\": %.2f, \"icon\": \"compass-rose\"}, "
           "{\"Gyro_Y\": %.2f, \"icon\": \"compass-rose\"}, "
           "{\"Gyro_Z\": %.2f, \"icon\": \"compass-rose\"},"
           "{\"Accel_X\": %.2f, \"icon\": \"compass-rose\"}, "
           "{\"Accel_Y\": %.2f, \"icon\": \"compass-rose\"}, "
           "{\"Accel_Z\": %.2f, \"icon\": \"compass-rose\"}]",
           ambientTemp, objectTemp, gyroX, gyroY, gyroZ, accelX, accelY, accelZ);

  // Print JSON string via Bluetooth
  SerialBT.println(jsonString);
}


bool initializeMLX90614() {
  if (!mlx.begin()) {
    return false;
  }
  Serial.println("MLX90614 initialized successfully.");
  return true;
}

bool initializeMPU6050() {
  if (mpu.begin() != 0) {
    return false;
  }
  mpu.calcGyroOffsets();
  Serial.println("MPU6050 initialized successfully.");
  return true;
}

bool initializeTFT() {
  tft.init();
  tft.setRotation(1); // Set rotation for horizontal display
  tft.fillScreen(TFT_BLACK); // Clear the screen with black color
  return true;
}

void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}
