// #include <Arduino.h>
// #include <Wire.h>
// #include <I2CKeyPad.h>

// // Function Prototypes
// void startDispensing();

// // I2C Keypad setup
// const uint8_t KEYPAD_ADDRESS = 0x20;
// I2CKeyPad keyPad(KEYPAD_ADDRESS);
// char keymap[17] = "123A456B789C*0#D";

// // Ultrasonic sensor pins
// const int trigPin = 15;  // D8
// const int echoPin = 13;  // RX
// const int maxDistance = 10; // Cup detection range

// // Relays for valve and water pump
// const int valvePin = 14;  // D5 (GPIO14) - Controls valve
// const int pumpPin = 12;   // D6 (GPIO12) - Controls water pump

// // Water Flow Sensor
// const int flowSensorPin = 2; // GPIO2
// volatile int pulseCount = 0;
// float calibrationFactor = 7.5;
// float dispensedLiters = 0;

// // User Input Handling
// unsigned long lastKeyPress = 0;
// const unsigned long debounceTime = 300;
// bool keyWasPressed = false;
// String keyBuffer = "";
// int state = 0;
// float waterPrice = 300;
// float totalAmount = 0;
// float waterLiters = 0;
// String phoneNumber = "";
// char inputMode = ' ';

// // Ultrasonic sensor function
// float readDistance() {
//   digitalWrite(trigPin, LOW);
//   delayMicroseconds(2);
//   digitalWrite(trigPin, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(trigPin, LOW);

//   long duration = pulseIn(echoPin, HIGH);
//   return duration * 0.034 / 2;
// }

// // Interrupt function for water flow sensor
// void IRAM_ATTR countPulses() {
//   pulseCount++;
// }

// void setup() {
//   pinMode(valvePin, OUTPUT);
//   pinMode(pumpPin, OUTPUT);
//   digitalWrite(valvePin, HIGH);
//   digitalWrite(pumpPin, LOW);
  
//   Serial.begin(115200);
  
//   Wire.begin(D2, D1);
//   Wire.setClock(100000);

//   pinMode(trigPin, OUTPUT);
//   pinMode(echoPin, INPUT);

//   pinMode(flowSensorPin, INPUT_PULLUP);
//   attachInterrupt(digitalPinToInterrupt(flowSensorPin), countPulses, RISING);

//   if (keyPad.begin()) {
//     Serial.println("\nWater Vending Machine");
//     Serial.println("Select input mode:");
//     Serial.println("A: Enter amount in RWF");
//     Serial.println("B: Enter liters of water");
//   } else {
//     Serial.println("Keypad initialization failed");
//   }
// }

// void loop() {
//   unsigned long currentTime = millis();

//   if (keyPad.isPressed()) {
//     if (!keyWasPressed && (currentTime - lastKeyPress > debounceTime)) {
//       uint8_t idx = keyPad.getKey();
//       if (idx < 16) {
//         char key = keymap[idx];
//         lastKeyPress = currentTime;
//         keyWasPressed = true;

//         if (key == '*') {
//           keyBuffer = "";
//           Serial.println("Input cleared");
//         } 
//         else if (key == '#') {
//           if (state == 1) {
//             float input = keyBuffer.toFloat();
//             if (input > 0) {
//               if (inputMode == 'A') {
//                 totalAmount = input;
//                 waterLiters = totalAmount / waterPrice;
//                 Serial.printf("Amount: %.2f RWF\n", totalAmount);
//                 Serial.printf("Water: %.2f Liters\n", waterLiters);
//               } else {
//                 waterLiters = input;
//                 totalAmount = waterLiters * waterPrice;
//                 Serial.printf("Water: %.2f Liters\n", waterLiters);
//                 Serial.printf("Cost: %.2f RWF\n", totalAmount);
//               }
//               Serial.println("Enter phone number:");
//               state = 2;
//             }
//             keyBuffer = "";
//           }
//           else if (state == 2) {
//             phoneNumber = keyBuffer;
//             Serial.println("Processing payment for: " + phoneNumber);
//             Serial.println("Payment completed!");
//             Serial.println("\nPLEASE PLACE YOUR CUP");
//             state = 3;
//             keyBuffer = "";

//             while (state == 3) {
//               float distance = readDistance();
//               if (distance <= maxDistance) {
//                 Serial.printf("Cup detected at %.1f cm\n", distance);
//                 startDispensing();  // Now correctly recognized
//                 state = 0;
//                 keyBuffer = "";
//                 inputMode = ' ';
//                 Serial.println("\nSelect input mode:");
//                 Serial.println("A: Enter amount in RWF");
//                 Serial.println("B: Enter liters of water");
//               } else {
//                 Serial.printf("Waiting for cup... Place within %.1f cm\n", (float)maxDistance);
//                 delay(500);
//               }
//             }
//           }
//         }
//         else if (state == 0 && (key == 'A' || key == 'B')) {
//           state = 1;
//           inputMode = key;
//           Serial.println(key == 'A' ? "Enter amount in RWF:" : "Enter liters of water:");
//         }
//         else {
//           keyBuffer += key;
//           Serial.printf("Input: %s\n", keyBuffer.c_str());
//         }
//       }
//     }
//   } else {
//     keyWasPressed = false;
//   }
//   delay(50);
// }

// // Function to control dispensing with correct relay timing
// void startDispensing() {
//   pulseCount = 0;
//   dispensedLiters = 0;
  
//   Serial.println("Starting water dispensing...");

//   // Open Pump First
//   digitalWrite(pumpPin, HIGH);
//   Serial.println("Water pump started...");
//   delay(1000); // Wait 1 second before opening valve

//   // Then Open Valve
//   digitalWrite(valvePin, LOW);
//   Serial.println("Valve opened, dispensing water...");

//   while (dispensedLiters < waterLiters) {
//     noInterrupts();
//     float litersDispensedNow = (pulseCount / calibrationFactor);
//     interrupts();
    
//     dispensedLiters = litersDispensedNow;
//     Serial.printf("Dispensed: %.2f / %.2f Liters\n", dispensedLiters, waterLiters);
    
//     if (readDistance() > maxDistance) {
//       Serial.println("Cup removed! Pausing...");
//       digitalWrite(valvePin, LOW);  // Close Valve first
//       delay(1000);                  // Wait 1 second
//       digitalWrite(pumpPin, LOW);    // Then Stop Pump
//       Serial.println("Paused due to cup removal. Waiting...");
      
//       while (readDistance() > maxDistance) delay(100);
      
//       Serial.println("Cup detected! Resuming...");
//       digitalWrite(pumpPin, HIGH);  // Start pump first
//       delay(1000);
//       digitalWrite(valvePin, LOW); // Open valve after 1 sec
//     }
//   }
  
//   // Close Valve first, then Pump
//   Serial.println("Closing valve...");
//   digitalWrite(valvePin, HIGH);
//   delay(1000);  // Wait 1 second before stopping pump
//   Serial.println("Stopping water pump...");
//   digitalWrite(pumpPin, LOW);
  
//   Serial.println("Dispensing complete!");
//   Serial.println("Thank you for using our service");
// }
