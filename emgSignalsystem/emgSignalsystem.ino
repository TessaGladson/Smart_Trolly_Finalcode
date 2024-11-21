#include <LiquidCrystal.h>

const int emgPin = A0;               // EMG signal pin (Analog Pin A0)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Initialize the LCD

// Define switch pins
#define Display1 7   // Display muscle activity level
#define Display2 8   // Display real-time intensity bar graph

// Thresholds and levels
int weakThreshold = 300;
int moderateThreshold = 600;
int strongThreshold = 800;

// Variables
int activityLevel = 0;
int emgSignal = 0;                 // EMG signal value
int displayMode = 1;               // Tracks the current display mode (1 or 2)
int lastMode = 0;                  // Previous display mode to detect changes

void setup() {
  Serial.begin(9600);              // For serial output
  lcd.begin(16, 2);                // Initialize LCD with 16 columns and 2 rows
  
  // Set switch pins as input with internal pull-up resistors
  pinMode(Display1, INPUT_PULLUP);
  pinMode(Display2, INPUT_PULLUP);
  
  // Initial welcome message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WELCOME TO EMG");
  lcd.setCursor(0, 1);
  lcd.print("MONITOR SYSTEM");
  delay(3000);
  lcd.clear();
}

void loop() {
  // Read the actual EMG signal from the analog input pin
  emgSignal = analogRead(emgPin);
  Serial.println(emgSignal);        // Send the signal to Serial Monitor
  
  // Detect display mode based on button press
  if (digitalRead(Display1) == LOW) { displayMode = 1; delay(200); }
  else if (digitalRead(Display2) == LOW) { displayMode = 2; delay(200); }

  // Only update the LCD display if the display mode changes
  if (displayMode != lastMode) {
    lcd.clear();
    lastMode = displayMode;
  }

  // Determine muscle activity level
  if (emgSignal > strongThreshold) {
    activityLevel = 3; // Strong
  } else if (emgSignal > moderateThreshold) {
    activityLevel = 2; // Moderate
  } else if (emgSignal > weakThreshold) {
    activityLevel = 1; // Weak
  } else {
    activityLevel = 0; // No activity
  }

  // Display on LCD based on the selected mode
  switch (displayMode) {
    case 1:
      // Muscle Activity Level
      lcd.setCursor(0, 0);
      lcd.print("Muscle Activity:");
      lcd.setCursor(0, 1);
      if (activityLevel == 3) {
        lcd.print("Strong");
      } else if (activityLevel == 2) {
        lcd.print("Moder.");
      } else if (activityLevel == 1) {
        lcd.print("Weak..");
      } else {
        lcd.print("None..");
      }
      Serial.print("Activity Level: "); Serial.println(activityLevel);
      break;
      
    case 2:
      // Real-time Intensity Bar Graph
      lcd.setCursor(0, 0);
      lcd.print("EMG Intensity:");
      lcd.setCursor(0, 1);
      int barLength = map(emgSignal, 0, 1023, 0, 16); // Map EMG to bar length
      for (int i = 0; i < barLength; i++) {
        lcd.print("|");
      }
      Serial.print("Intensity: "); Serial.println(emgSignal);
      break;
  }
  
  delay(500); // Delay for readability
}
