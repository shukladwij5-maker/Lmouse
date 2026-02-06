#include <ArduinoBLE.h>

// --- Pin and Configuration Definitions ---
const int joyX = A0; 
const int joyY = A1;
const int joyButton = 2; 

// Initial center reading (calibrated in setup)
int centerX = 0; 
int centerY = 0; 

// Sensitivity/Speed Control (Adjust these values)
const int threshold = 100; // Joystick deadzone
const int moveSpeed = 5;   // Higher value = Faster cursor movement

// Button state tracking
int lastButtonState = HIGH; 
const int debounceDelay = 50; 

// --- Bluetooth Low Energy HID Definitions ---
// The UNO R4 WiFi uses the standard BLE HID Service
BLEService hidService("1812"); 

// The Report Characteristic is used to send mouse movement/button data
// Standard UUID for HID Report Characteristic
BLECharacteristic reportCharacteristic("2A4D", BLEWriteWithoutResponse, 8, false); 

void setup() {
  Serial.begin(9600);
  // while (!Serial); // Uncomment for debugging wait

  pinMode(joyButton, INPUT_PULLUP); 

  // Read and store the initial center position of the joystick for calibration
  centerX = analogRead(joyX);
  centerY = analogRead(joyY);
  
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  // Set the advertised local name and service UUID
  BLE.setLocalName("R4BLEMouse");
  BLE.setAdvertisedService(hidService);

  // Add the HID Service and Report Characteristic
  hidService.addCharacteristic(reportCharacteristic);
  BLE.addService(hidService);

  // Start advertising the BLE device
  BLE.advertise();
  Serial.println("BLE Mouse Advertising...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    // Loop while the central device is connected
    while (central.connected()) {
      handleMouseMovement();
      handleButtonClick();
      delay(5); // Small delay for stable operation
    }
    
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
    // Restart advertising after disconnection
    BLE.advertise();
  }
}

void handleMouseMovement() {
  int xValue = analogRead(joyX);
  int yValue = analogRead(joyY);
  
  int dx = xValue - centerX;
  int dy = yValue - centerY;
  
  int mouseX = 0;
  int mouseY = 0;
  
  // X-axis movement
  if (abs(dx) > threshold) {
    // Map the joystick range outside the threshold to the mouse movement speed
    // Use the moveSpeed variable for sensitivity
    mouseX = map(dx, -1023, 1023, -moveSpeed, moveSpeed);
  }
  
  // Y-axis movement (Note: Mouse Y is often inverted from joystick reading)
  if (abs(dy) > threshold) {
    // Invert the result for typical mouse behavior (pushing up = negative Y)
    mouseY = map(dy, -1023, 1023, moveSpeed, -moveSpeed);
  }

  // Send movement report if there is any movement
  if (mouseX != 0 || mouseY != 0) {
    // Mouse movement report format:
    // [Buttons (1 byte), X-axis (1 byte), Y-axis (1 byte), Scroll (1 byte)]
    // We only send X and Y relative movement
    
    // Create the 4-byte report
    // The report needs to be a 4-byte array for most BLE HID stacks. 
    // We are only concerned with bytes 1 and 2 for movement (X and Y).
    // The report characteristic is actually 8 bytes, but often only 4 are used for mouse movement. 
    // NOTE: This simple 4-byte report may need adjustment based on your OS/device's BLE HID requirements.
    uint8_t report[4] = {0x00, (uint8_t)mouseX, (uint8_t)mouseY, 0x00}; 
    reportCharacteristic.writeValue(report, 4);
  }
}

void handleButtonClick() {
  int reading = digitalRead(joyButton);

  if (reading != lastButtonState) {
    delay(debounceDelay);
    reading = digitalRead(joyButton);
    
    uint8_t buttonReport[4] = {0x00, 0x00, 0x00, 0x00};

    // Check for a press (LOW when pressed due to INPUT_PULLUP)
    if (reading == LOW) {
      buttonReport[0] = 0x01; // Left button bit (0x01)
      reportCharacteristic.writeValue(buttonReport, 4);
      // Serial.println("BLE Left Click Pressed");
    } 
    // Check for a release (HIGH when released)
    else if (reading == HIGH) {
      buttonReport[0] = 0x00; // No buttons pressed
      reportCharacteristic.writeValue(buttonReport, 4);
      // Serial.println("BLE Left Click Released");
    }
  }
  lastButtonState = reading;
}
