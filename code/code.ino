#include <Wire.h> // Include Wire library for I2C communication
#include <LiquidCrystal_I2C.h> // Include library for I2C LCD
#include <Keypad.h> // Include library for keypad input
#include <Servo.h> // Include library for servo motor control

#define PASSWORD_LENGTH 5  // Define the password length (4 digits + null terminator)
#define ROWS 4  // Define number of keypad rows
#define COLS 4  // Define number of keypad columns
#define RED_PIN 12    // Define RGB LED Red pin for wrong password
#define GREEN_PIN 10  // Define RGB LED Green pin for correct password

// Initialize 16x2 LCD with I2C address 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Create servo object for controlling the door lock
Servo doorServo;

// Define buzzer and button pins
#define BUZZER_PIN 11
#define BUTTON_PIN 2  // Push button pin

// Define the correct password
const char password[PASSWORD_LENGTH] = "2356";
char inputPassword[PASSWORD_LENGTH] = ""; // Array to store entered password
byte inputIndex = 0; // Index for input array

// Define keypad characters
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Define row and column pin connections
byte rowPins[ROWS] = {6, 7, 8, 9};  
byte colPins[COLS] = {2, 3, 4, 5};  

// Initialize keypad with keymap and pin configuration
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
    Serial.begin(9600); // Initialize serial communication

    lcd.init(); // Initialize LCD
    lcd.backlight(); // Turn on LCD backlight

    doorServo.attach(10); // Attach servo to pin 10
    lockDoor(); // Ensure the door is locked at the start

    pinMode(BUZZER_PIN, OUTPUT); // Set buzzer as output
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Set button as input with pull-up resistor
    pinMode(RED_PIN, OUTPUT);  // Set RGB Red pin as output
    pinMode(GREEN_PIN, OUTPUT);  // Set RGB Green pin as output

    // Ensure LEDs are off initially
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);

    lcd.setCursor(0, 0); // Set cursor to first row, first column
    lcd.print("Door Locked"); // Display initial message
}

void loop() {
    // Check if button is pressed
    if (digitalRead(BUTTON_PIN) == LOW) { 
        lcd.clear(); // Clear LCD screen
        lcd.setCursor(0, 0);
        lcd.print("Enter Password:"); // Prompt for password

        while (true) {
            checkKeypad(); // Continuously check for keypad input
        }
    }
}

void checkKeypad() {
    char key = keypad.getKey(); // Get key press
    if (key) { // If a key is pressed
        Serial.print("Key Pressed: ");
        Serial.println(key); // Log key press

        lcd.setCursor(0, 1); // Set cursor to second row
        lcd.print("Key: ");
        lcd.print(key); // Display the key pressed
        delay(500); // Short delay

        if (key == '#') {  // Confirm key
            inputPassword[inputIndex] = '\0';  // Null-terminate string

            Serial.print("Entered Password: ");
            Serial.println(inputPassword); // Log entered password

            if (strcmp(inputPassword, password) == 0) { // Compare entered password
                digitalWrite(GREEN_PIN, HIGH);  // Turn on green light
                digitalWrite(RED_PIN, LOW);    // Turn off red light
                tone(BUZZER_PIN, 1000, 500);  // Play a tone at 1000 Hz for 500 ms
                unlockDoor(); // Unlock door
            } else { // If password is incorrect
                lcd.setCursor(0, 1);
                lcd.print("Access Denied!"); // Display denial message
                digitalWrite(RED_PIN, HIGH);  // Turn on red light
                digitalWrite(GREEN_PIN, LOW); // Turn off green light
                tone(BUZZER_PIN, 500, 500);   // Play error tone
                delay(1000); // Short delay
            }
            resetInput(); // Reset password input
        } else if (key == '*') {  // Reset entry
            resetInput();
        } else { // Add key to input password
            if (inputIndex < PASSWORD_LENGTH - 1) { // Ensure within limit
                inputPassword[inputIndex] = key; // Store key
                inputIndex++; // Increment index
            }
        }

        lcd.setCursor(0, 1);
        lcd.print("              "); // Clear previous input
    }
}

void unlockDoor() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Granted!"); // Display success message
    Serial.println("Access Granted!");

    digitalWrite(BUZZER_PIN, HIGH); // Turn on buzzer
    delay(500);
    digitalWrite(BUZZER_PIN, LOW); // Turn off buzzer

    // Rotate servo to unlock position
    for (int pos = 0; pos <= 90; pos++) {
        doorServo.write(pos);
        delayMicroseconds(80);
    }
    delay(100);

    // Rotate servo back to lock position
    for (int pos = 90; pos >= 0; pos--) {
        doorServo.write(pos);
        delay(20);
    }

    while (digitalRead(BUTTON_PIN) == HIGH) { // Wait for button press
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Door Locked");
    }
    lockDoor();
}

void lockDoor() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door Locked"); // Display locked message
    Serial.println("Door Locked!");
    doorServo.write(0); // Set servo to locked position
}

void resetInput() {
    inputIndex = 0; // Reset input index
    lcd.clear();
    lcd.setCursor(0, 0);
    if (digitalRead(BUTTON_PIN) != LOW) { 
        lcd.print("Enter Password:"); // Prompt for password
        digitalWrite(GREEN_PIN, LOW); // Turn off green light
        digitalWrite(RED_PIN, LOW);   // Turn off red light
        noTone(BUZZER_PIN); // Turn off buzzer
        while (true) {
            checkKeypad(); // Keep checking for input
        }
    }
}
