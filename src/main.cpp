#include <Arduino.h>
#include <Wire.h>
#include <I2CKeyPad.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

#define SDA_PIN D2
#define SCL_PIN D1 

// Add WiFi credentials
const char* ssid = "Move";
const char* password = "11111111";
const char* serverUrl = "http://192.168.137.68:5000/api/record-transaction";

// Function Prototypes
void startDispensing();
void increase();
bool sendTransactionData();

// Initialize LCD with correct constructor on the same I2C bus
LiquidCrystal_I2C lcd(0x27, 20, 4);

// I2C Keypad setup on default Wire bus
const uint8_t KEYPAD_ADDRESS = 0x20;
I2CKeyPad keyPad(KEYPAD_ADDRESS, &Wire);
char keymap[17] = "123A456B789C*0#D";

// Ultrasonic sensor pins
const int trigPin = 15;  // D8
const int echoPin = 13;  // D7 (GPIO13)
const int maxDistance = 5; // Cup detection range

// led red pin on D3
const int ledRed = D3; // D3
// led green pin on D6
const int ledGreen = D6; // D6
void indicateSuccess();
void indicateError();
// Relays for valve and water pump
const int valvePin = 14;  // D5 (GPIO14) - Controls valve

// Water Flow Sensor
const int sensorPin = 2; // GPIO2
volatile long pulse = 0;
float volume;
const float calibrationFactor = 7.5; // Adjust this value based on testing
float lastVolume = 0;

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

// reset function
void resetESP() {
    Serial.println("Performing hardware reset...");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Resetting System");
    ESP.restart();  // Hardware reset of ESP8266
    delay(1000);
    
}

void setup() {
  Serial.begin(115200);
// Add watchdog timer
    ESP.wdtDisable();
    ESP.wdtEnable(WDTO_8S);
     // Optional: Force reset after power up
    // resetESP();
  WiFi.begin(ssid, password);

  pinMode(valvePin, OUTPUT);
  digitalWrite(valvePin, HIGH);


  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, LOW);

  pinMode(sensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensorPin), increase, RISING);

  // Initialize a single I2C bus for both LCD and Keypad
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  lcd.init();                
  lcd.backlight();
  lcd.clear();

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
    lcd.setCursor(0,1);
    lcd.print("Reconnected to WiFi");
    indicateSuccess();
    delay(1000);

  }

  if (keyPad.begin()) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("===Welcome to===");
    lcd.setCursor(0,1);
    lcd.print("Water Vending Machine");
    lcd.setCursor(0,2);
    lcd.print("================");
    delay(3000);

    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print(" Type Input:");
    lcd.setCursor(0,1);
    lcd.print("A: Amount in RWF");
    lcd.setCursor(0,2);
    lcd.print("B: Water in mL");
    lcd.setCursor(0,3);
    lcd.print("Press A or B to start");
  } else {
    Serial.println("Keypad initialization failed");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Keypad init failed");
    indicateError();
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
          delay(1000);
          
          if (state == 1) {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(inputMode == 'A' ? "Enter amount in RWF:" : "Enter ml of water:");
          }
        } 
        else if (key == '#') {
          if (keyBuffer.length() > 0) {
            keyBuffer = keyBuffer.substring(0, keyBuffer.length() - 1);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Input: ");
            lcd.print(keyBuffer);
          }
        }
        else if (key == 'C') {
          if (state == 1) {
            float input = keyBuffer.toFloat();
            if (input > 0) {
              if (inputMode == 'A') {
                totalAmount = input;
                waterMilliliters = totalAmount / waterPrice;
              } else {
                waterMilliliters = input;
                totalAmount = waterMilliliters * waterPrice;
              }
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Amount: ");
              lcd.print(totalAmount);
              lcd.print(" RWF");
              lcd.setCursor(0,1);
              lcd.print("Water: ");
              lcd.print(waterMilliliters);
              lcd.print(" mL");
              delay(2000);
              
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Enter phone number:");
              state = 2;
            }
            keyBuffer = "";
          } 
          else if (state == 2) {
            if (keyBuffer.length() == 10) {
              phoneNumber = keyBuffer;
              state = 3;
              keyBuffer = "";

              if (sendTransactionData()) {
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Processing payment....!");
                indicateSuccess();
                delay(2000);
                lcd.setCursor(0,1);
                lcd.print("Payment successful!");
                Serial.println("Payment successful!");

                indicateSuccess();
                
                while (state == 3) {
                  float distance = readDistance();
                  if (distance <= maxDistance) {
                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print("Cup detected");
                    lcd.setCursor(0,1);
                    lcd.print("Distance: ");
                    lcd.print(distance);
                    lcd.print("cm");
                    startDispensing();
                    indicateSuccess();
                    state = 0;
                    keyBuffer = "";
                    inputMode = ' ';
                    break;
                  } else {
                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print("Place cup below");
                    lcd.setCursor(0,1);
                    lcd.print("Distance: ");
                    lcd.print(distance);
                    lcd.print("cm");
                    delay(1000);
                  }
                }
              } else {
                Serial.println("Payment failed");
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Payment failed");
                lcd.setCursor(0,1);
                lcd.print("Try again");
                indicateError();
                state = 0;
                delay(2000);
              }
            } else {
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Invalid number");
              lcd.setCursor(0,1);
              lcd.print("Must be 10 digits");
              delay(2000);
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Enter phone number:");
              keyBuffer = "";
            }
          }
        }
        else if (key == 'D') {
          state = 0;
          keyBuffer = "";
          inputMode = ' ';
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Choose Input:");
          lcd.setCursor(0,1);
          lcd.print("A: Amount in RWF");
          lcd.setCursor(0,2);
          lcd.print("B: Water in mL");
          lcd.setCursor(0,3);
          lcd.print("Press A or B");
        }
        else if (state == 0 && (key == 'A' || key == 'B')) {
          state = 1;
          inputMode = key;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(key == 'A' ? "Enter amount in RWF:" : "Enter ml of water:");
        }
        else {
          keyBuffer += key;
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
  unsigned long startTime = millis();
  unsigned long lastPulseTime = startTime;
  unsigned long lastFlowCheck = startTime;
  const unsigned long TIMEOUT_MS = 30000; // 30 second timeout
  const unsigned long FLOW_CHECK_INTERVAL = 2000; // Check flow every 2 seconds
  const float MIN_FLOW_RATE = 0.5; // Minimum flow rate in mL/second

  digitalWrite(valvePin, LOW);  // Open valve
  Serial.println("Starting dispensing...");

  while (true) {
    unsigned long currentTime = millis();
    
    // Timeout protection
    // if (currentTime - startTime > TIMEOUT_MS) {
    //   digitalWrite(valvePin, HIGH);
    //   lcd.setCursor(0, 3);
    //   lcd.print("Status: Timeout Error");
    //   indicateError();
    //   delay(2000);
    //   break;
    // }

    // Calculate volume with new calibration
    volume = (pulse / calibrationFactor) * 1000 / 60;
    
    // Flow rate monitoring
    if (currentTime - lastFlowCheck >= FLOW_CHECK_INTERVAL) {
      float flowRate = (volume - lastVolume) / ((currentTime - lastFlowCheck) / 1000.0);
      lastVolume = volume;
      lastFlowCheck = currentTime;
      
      // if (flowRate < MIN_FLOW_RATE && volume < waterMilliliters) {
      //   lcd.setCursor(0, 3);
      //   lcd.print("Status: Low Flow    ");
      //   indicateError();
      //   delay(1000);
      // }
    }

    // Flow sensor error detection
    // if (pulse > 0 && currentTime - lastPulseTime > 5000) {
    //   digitalWrite(valvePin, HIGH);
    //   lcd.setCursor(0, 3);
    //   lcd.print("Status: Sensor Error");
    //   indicateError();
    //   delay(2000);
    //   break;
    // }

    // Update display
      lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dispensing Water");
  lcd.setCursor(0, 1);
  lcd.print("Spensing: ");
  lcd.print(volume);
  lcd.print(" mL");
  lcd.setCursor(0, 2);
  lcd.print("Target: ");
  lcd.print(waterMilliliters);
  lcd.print(" mL");
  lcd.setCursor(0, 3);
  lcd.print("Status: Running");
    
    // Target volume reached
    if (volume >= waterMilliliters) {
      digitalWrite(valvePin, HIGH);
      lcd.setCursor(0, 3);
      lcd.print("Status: Complete    ");
      indicateSuccess();
      break;
    }

    delay(100);
  }

  // Reset system state
  digitalWrite(valvePin, HIGH);
  pulse = 0;
  volume = 0;
}



// send transaction data:

bool sendTransactionData() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("WiFi not connected");
        for (int i = 0; i < 5; i++) {
          digitalWrite(ledRed, HIGH);
          delay(500);
          digitalWrite(ledRed, LOW);
          delay(500);
        }
        return false;
    }
    
    HTTPClient http;
    WiFiClient client;
    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    // Format phone number with country code if not present
    String formattedPhone = phoneNumber;
    if (!phoneNumber.startsWith("+25")) {
        formattedPhone = "+25" + phoneNumber;
    }
    
    JsonDocument doc;
    doc["phone_number"] = formattedPhone;
    doc["liters_dispensed"] = waterMilliliters/1000.0;
    doc["amount_paid"] = totalAmount;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    int httpResponseCode = http.POST(jsonString);
    http.end();
    
    return httpResponseCode > 0;
}


void indicateSuccess() {
  digitalWrite(ledGreen, HIGH);
  delay(2000);
  digitalWrite(ledGreen, LOW);
}

void indicateError() {
  digitalWrite(ledRed, HIGH);
  delay(2000);
  digitalWrite(ledRed, LOW);
}