#include <Mouse.h>

// --- Pin Definitions ---
// Joystick analog pins for X and Y movement
const int joyX = A0; 
const int joyY = A1;

// Joystick button pin for Left Click
const int joyButton = 2; 

// --- Variables to Store Joystick State ---
int centerX; // Stores the initial center value for X axis
int centerY; // Stores the initial center value for Y axis

// --- Configuration ---
// Threshold for movement sensitivity (how far off center to trigger movement)
const int threshold = 100; 

// Speed factor for cursor movement (higher = faster)
const int moveSpeed = 3; 

// Button state tracking
int lastButtonState = HIGH; 
const int debounceDelay = 50; // Debounce time in milliseconds

void setup() {
  // Initialize the digital pin for the button as an input with a Pull-Up resistor
  // The internal pull-up means the button reads HIGH when not pressed, and LOW when pressed.
  pinMode(joyButton, INPUT_PULLUP); 

  // Start the USB Mouse communication
  Mouse.begin(); 
  
  // Read and store the initial center position of the joystick
  centerX = analogRead(joyX);
  centerY = analogRead(joyY);
  
  // Optional: Start serial communication for debugging
  // Serial.begin(9600);
  // Serial.print("Center X: "); Serial.println(centerX);
  // Serial.print("Center Y: "); Serial.println(centerY);
}

void loop() {
  // --- 1. Mouse Movement (Joystick) ---
  
  // Read current analog values
  int xValue = analogRead(joyX);
  int yValue = analogRead(joyY);
  
  // Calculate the difference from the center
  int dx = xValue - centerX;
  int dy = yValue - centerY;
  
  // Variable to store the final cursor movement
  int mouseX = 0;
  int mouseY = 0;
  
  // Check X-axis movement
  if (dx > threshold) {
    // Moved right: Cursor moves right (positive X)
    // The amount moved is proportional to how far you push it past the center
    mouseX = map(dx, threshold, 1023, 0, moveSpeed);
  } else if (dx < -threshold) {
    // Moved left: Cursor moves left (negative X)
    mouseX = map(dx, -1023, -threshold, -moveSpeed, 0);
  }
  
  // Check Y-axis movement (Note: USB mouse Y-axis is inverted from joystick reading)
  if (dy > threshold) {
    // Moved up on joystick (lower Y reading): Cursor moves up (negative Y)
    mouseY = map(dy, threshold, 1023, 0, -moveSpeed);
  } else if (dy < -threshold) {
    // Moved down on joystick (higher Y reading): Cursor moves down (positive Y)
    mouseY = map(dy, -1023, -threshold, moveSpeed, 0);
  }
  
  // Send the movement command if there is any movement
  if (mouseX != 0 || mouseY != 0) {
    Mouse.move(mouseX, mouseY);
  }
  
  // --- 2. Button Press (Left Click) ---
  
  // Read the current state of the button
  int reading = digitalRead(joyButton);

  // Check if the state has changed (LOW means pressed, since we used INPUT_PULLUP)
  if (reading != lastButtonState) {
    // Debounce: Wait a little bit to filter out noise
    delay(debounceDelay);
    
    // Read the state again to confirm the press/release
    reading = digitalRead(joyButton);
    
    // Check for a press (state change from HIGH to LOW)
    if (reading == LOW) {
      Mouse.press(MOUSE_LEFT);
      // Serial.println("Left Click Pressed");
    } 
    // Check for a release (state change from LOW to HIGH)
    else if (reading == HIGH) {
      Mouse.release(MOUSE_LEFT);
      // Serial.println("Left Click Released");
    }
  }

  // Save the current reading as the last state
  lastButtonState = reading;

  // Small delay to prevent the loop from running too fast
  delay(5); 
}
