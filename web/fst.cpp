#include <Arduino.h>
#include <Wire.h>
#include <I2CKeyPad.h>

// Function Prototypes
void startDispensing();
void increase();

// I2C Keypad setup
const uint8_t KEYPAD_ADDRESS = 0x20;
I2CKeyPad keyPad(KEYPAD_ADDRESS);
char keymap[17] = "123A456B789C*0#D";

// Ultrasonic sensor pins
const int trigPin = 15;  // D8
const int echoPin = 13;  // RX
const int maxDistance = 10; // Cup detection range

// Relays for valve and water pump
const int valvePin = 14;  // D5 (GPIO14) - Controls valve
const int pumpPin = 12;   // D6 (GPIO12) - Controls water pump

// Water Flow Sensor
const int sensorPin = 2; // GPIO2
volatile long pulse = 0;
float volume = 0;

// User Input Handling
unsigned long lastKeyPress = 0;
const unsigned long debounceTime = 300;
bool keyWasPressed = false;
String keyBuffer = "";
int state = 0;
float waterPrice = 0.3; // Price per mL
float totalAmount = 0;
float waterMilliliters = 0;
String phoneNumber = "";
char inputMode = ' ';

// Ultrasonic sensor function
float readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

// Interrupt function for water flow sensor
void IRAM_ATTR increase() {
  pulse++;
}

void setup() {
  pinMode(valvePin, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(valvePin, HIGH);
  digitalWrite(pumpPin, LOW);
  
  Serial.begin(115200);
  
  Wire.begin(D2, D1);
  Wire.setClock(100000);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(sensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensorPin), increase, RISING);

  if (keyPad.begin()) {
    Serial.println("\nWater Vending Machine");
    Serial.println("Select input mode:");
    Serial.println("A: Enter amount in RWF");
    Serial.println("B: Enter milliliters of water");
  } else {
    Serial.println("Keypad initialization failed");
  }
}

void loop() {
  unsigned long currentTime = millis();

  if (keyPad.isPressed()) {
    if (!keyWasPressed && (currentTime - lastKeyPress > debounceTime)) {
      uint8_t idx = keyPad.getKey();
      if (idx < 16) {
        char key = keymap[idx];
        lastKeyPress = currentTime;
        keyWasPressed = true;

        if (key == '*') {
          keyBuffer = "";
          Serial.println("Input cleared");
        } 
        else if (key == '#') {
          if (state == 1) {
            float input = keyBuffer.toFloat();
            if (input > 0) {
              if (inputMode == 'A') {
                totalAmount = input;
                waterMilliliters = totalAmount / waterPrice;
                Serial.printf("Amount: %.2f RWF\n", totalAmount);
                Serial.printf("Water: %.2f mL\n", waterMilliliters);
              } else {
                waterMilliliters = input;
                totalAmount = waterMilliliters * waterPrice;
                Serial.printf("Water: %.2f mL\n", waterMilliliters);
                Serial.printf("Cost: %.2f RWF\n", totalAmount);
              }
              Serial.println("Enter phone number:");
              state = 2;
            }
            keyBuffer = "";
          }
          else if (state == 2) {
            phoneNumber = keyBuffer;
            Serial.println("Processing payment for: " + phoneNumber);
            Serial.println("Payment completed!");
            Serial.println("\nPLEASE PLACE YOUR CUP");
            state = 3;
            keyBuffer = "";

            while (state == 3) {
              float distance = readDistance();
              if (distance <= maxDistance) {
                Serial.printf("Cup detected at %.1f cm\n", distance);
                startDispensing();
                state = 0;
                keyBuffer = "";
                inputMode = ' ';
                Serial.println("\nSelect input mode:");
                Serial.println("A: Enter amount in RWF");
                Serial.println("B: Enter milliliters of water");
              } else {
                Serial.printf("Waiting for cup... Place within %.1f cm\n", (float)maxDistance);
                delay(500);
              }
            }
          }
        }
        else if (state == 0 && (key == 'A' || key == 'B')) {
          state = 1;
          inputMode = key;
          Serial.println(key == 'A' ? "Enter amount in RWF:" : "Enter milliliters of water:");
        }
        else {
          keyBuffer += key;
          Serial.printf("Input: %s\n", keyBuffer.c_str());
    
        }
      }
    }
  } else {
    keyWasPressed = false;
  }
  delay(50);
}

void startDispensing() {
  pulse = 0;
  volume = 0;
  
  Serial.println("Starting water dispensing...");

  digitalWrite(pumpPin, HIGH);
  Serial.println("Water pump started...");
  delay(1000);

  digitalWrite(valvePin, LOW);
  Serial.println("Valve opened, dispensing water...");

  while (volume < waterMilliliters) {

    volume = 2.663 * pulse;
    Serial.printf("Dispensed: %.2f / %.2f mL\n", volume, waterMilliliters);
    
    if (readDistance() > maxDistance) {
      Serial.println("Cup removed! Pausing...");
      digitalWrite(valvePin, HIGH);
      delay(1000);
      digitalWrite(pumpPin, LOW);
      Serial.println("Paused due to cup removal. Waiting..."); 
      
      while (readDistance() > maxDistance) delay(100);
      
      Serial.println("Cup detected! Resuming...");
      digitalWrite(pumpPin, HIGH);
      delay(1000);
      digitalWrite(valvePin, LOW);
    }
    delay(500);
  }
  
  Serial.println("Closing valve...");
  digitalWrite(valvePin, HIGH);
  delay(1000);
  Serial.println("Stopping water pump...");

  
  Serial.println("Dispensing complete!");
  Serial.println("Thank you for using our service");
}