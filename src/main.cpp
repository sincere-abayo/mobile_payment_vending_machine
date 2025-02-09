#include <Arduino.h>
#include <Wire.h>
#include <I2CKeyPad.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

// Create second wire instance for LCD
TwoWire wire2 = TwoWire();

#define SDA_PIN 12 
#define SCL_PIN 0 

// Add WiFi credentials
const char* ssid = "GIHANGA AI";
const char* password = "GIHANGA1";
const char* serverUrl = "http://192.168.1.26:5000/api/record-transaction";


// Function Prototypes
void startDispensing();
void increase();
bool sendTransactionData();

// Then initialize LCD with correct constructor
LiquidCrystal_I2C lcd(0x27, 20, 4);

// I2C Keypad setup on default Wire bus
const uint8_t KEYPAD_ADDRESS = 0x20;
I2CKeyPad keyPad(KEYPAD_ADDRESS, &Wire);
char keymap[17] = "123A456B789C*0#D";

// Ultrasonic sensor pins
const int trigPin = 15;  // D8
const int echoPin = 13;  // D7 (GPIO13)
const int maxDistance = 10; // Cup detection range

// Relays for valve and water pump
const int valvePin = 14;  // D5 (GPIO14) - Controls valve

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
   WiFi.begin(ssid, password);  
      
  pinMode(valvePin, OUTPUT);
  digitalWrite(valvePin, HIGH);
  
  Serial.begin(115200);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(sensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensorPin), increase, RISING);

    // Initialize second I2C bus for LCD
  wire2.begin(20, 4); // SDA, SCL for LCD
  wire2.setClock(100000);
    lcd.init();                /* initialize the lcd  */
      lcd.backlight();
      lcd.clear();
 
     // Initialize first I2C bus for keypad
    Wire.begin(D2, D1);  // SDA, SCL for keypad
    Wire.setClock(100000);


 if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost... Reconnecting...");
    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print("WiFi connecting....");   
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nReconnected to WiFi");
    delay(1000);
    Serial.println(".");
    lcd.setCursor(0,1);
    lcd.print("Reconnected to WiFi");
}
 if (keyPad.begin()) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("===Welcome to===");
    lcd.setCursor(0,1);
    lcd.print("Water Vending Machine");
    lcd.setCursor(0,2);
    lcd.print("================");
    delay(3000);  // Give time to read welcome message
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Select Input Mode:");
    lcd.setCursor(0,1);
    lcd.print("A: Amount in RWF");
    lcd.setCursor(0,2);
    lcd.print("B: Water in mL");
    lcd.setCursor(0,3);
    lcd.print("Press A or B to start");
  }
  else {
    Serial.println("Keypad initialization failed");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Keypad init failed");
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
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Input cleared");
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
                 lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Enter Value:");
                lcd.setCursor(0,1);
                lcd.print(keyBuffer);
                lcd.setCursor(0,3);
                lcd.print("# to confirm, * clear");
                delay(200);  // Smoother display updates
              }
               else {
                waterMilliliters = input;
                totalAmount = waterMilliliters * waterPrice;
                Serial.printf("Water: %.2f mL\n", waterMilliliters);
                Serial.printf("Cost: %.2f RWF\n", totalAmount);
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Water: ");
                lcd.print(waterMilliliters);
                lcd.setCursor(0,1);
                lcd.print("Cost: ");
                lcd.print(totalAmount);
              }
              Serial.println("Enter phone number:");
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Enter phone number:");
              state = 2;
            }
            keyBuffer = "";
          }
          else if (state == 2) {
            phoneNumber = keyBuffer;
            Serial.println("Processing payment for: " + phoneNumber);
          Serial.println("Total amount: " + String(totalAmount));     
                                 lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Processing Payment");
          lcd.setCursor(0,1);
          lcd.print("Phone: " + phoneNumber);
          lcd.setCursor(0,2);
          lcd.print("Amount: " + String(totalAmount));
          lcd.setCursor(0,3);
          lcd.print("Please wait...");
          delay(2000);
            state = 3;
            keyBuffer = "";
          // sentransaction data to server
        
    
          if (sendTransactionData()) {  // Modified to return bool
        Serial.println("Payment data transmitted successfully!");
        Serial.println("Payment completed!");
        Serial.println("\nPLEASE PLACE YOUR CUP");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Payment data transim");
        lcd.setCursor(0,1);
        lcd.print("itted successfully!");
        lcd.clear();
        lcd.setCursor(0,3);
        lcd.print("Payment completed!");
      

        state = 3;
        keyBuffer = "";
        // Continue with cup detection and dispensing
    } else {
        Serial.println("Payment data transmission failed. Please try again.");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Payment data transmission");
        lcd.setCursor(0,1);
        lcd.print("failed. Please try again.");
        state = 0;
    }

            while (state == 3) {
              float distance = readDistance();
              if (distance <= maxDistance) {
                Serial.printf("Cup detected at %.1f cm\n", distance);
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Place Your Cup");
                delay(2000);
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Cup detected at");
                lcd.setCursor(0,1);
                lcd.print(distance);
                startDispensing();
                state = 0;
                keyBuffer = "";
                inputMode = ' ';
                Serial.println("\nSelect input mode:");
                Serial.println("A: Enter amount in RWF");
                Serial.println("B: Enter milliliters of water");
                lcd.clear();
                lcd.setCursor(1,0);
                lcd.print("Select input mode:");
                delay(2000);
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("A: Enter amount in RWF");
                lcd.setCursor(0,1);
                lcd.print("B: Enter ml of water");
                delay(2000);
                    
              } else {
                Serial.printf("Waiting for cup... Place within %.1f cm\n", (float)maxDistance);
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Waiting for cup...");
                lcd.setCursor(0,1);
                lcd.print("Place within");
                lcd.setCursor(0,1);
                lcd.print(maxDistance);
                delay(1000);

              }
            }
          }
        }
        else if (state == 0 && (key == 'A' || key == 'B')) {
          state = 1;
          inputMode = key;
          Serial.println(key == 'A' ? "Enter amount in RWF:" : "Enter ml of water:");
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(key == 'A' ? "Enter amount in RWF:" : "Enter ml of water:");
          delay(2000);
        }
        else {
          keyBuffer += key;
          Serial.printf("Input: %s\n", keyBuffer.c_str());
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Input: ");
          lcd.print(keyBuffer);

    
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
  digitalWrite(valvePin, LOW);
  
  Serial.println("Starting water dispensing...");
 lcd.clear();
lcd.setCursor(0,0);
lcd.print("Dispensing Water");
lcd.setCursor(0,1);
lcd.print("Volume: " + String(volume));
lcd.setCursor(0,2);
lcd.print("Target: " + String(waterMilliliters));
lcd.setCursor(0,3);
lcd.print("Please wait...");
delay(500);


  while (volume < waterMilliliters) {

    volume = 2.663 * pulse;
    Serial.printf("Dispensed: %.2f / %.2f mL\n", volume, waterMilliliters);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Dispensed: ");
    lcd.print(volume);
    lcd.setCursor(0,1);
    lcd.print("Target: ");
    lcd.print(waterMilliliters);
    lcd.setCursor(0,2);
    lcd.print("Please wait...");
    lcd.setCursor(0,3);
 lcd.print("Volume: ");
    lcd.print(volume);
    
    
    if (readDistance() > maxDistance) {
      Serial.println("Cup removed! Pausing...");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Cup removed! Pausing...");
      digitalWrite(valvePin, HIGH);
      delay(1000);
      Serial.println("Paused due to cup removal. Waiting..."); 
        // lcd.clear();
        lcd.setCursor(1,0);
        lcd.print("Paused due to cup");
        lcd.setCursor(2,1);
        lcd.print("removal. Waiting...");
      
      while (readDistance() > maxDistance) delay(100);
      
      digitalWrite(valvePin, LOW);
      Serial.println("Cup detected! Resuming...");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Cup detected! Resuming...");
      delay(1000);
    }
    delay(500);
  }
  
  // sentransaction data to server
  // sendTransactionData();
  Serial.println("Closing valve...");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Closing valve...");
  digitalWrite(valvePin, HIGH);
  delay(1000);
  Serial.println("Stopping water pump...");

  Serial.println("Dispensing complete!");
  Serial.println("Thank you for using our service");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Dispensing complete!");
  lcd.setCursor(0,1);
  lcd.print("Thank you for using");
  lcd.setCursor(0,1);
  lcd.print("our service");
}

// send transaction data:

bool sendTransactionData() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("WiFi not connected");
        return false;
    }
    
    HTTPClient http;
    WiFiClient client;
    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    JsonDocument doc;
    doc["phone_number"] = phoneNumber;
    doc["liters_dispensed"] = waterMilliliters/1000;
    doc["amount_paid"] = totalAmount;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    int httpResponseCode = http.POST(jsonString);
    http.end();
    
    return httpResponseCode > 0;
}
