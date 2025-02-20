// #include <Arduino.h>
// #include <Wire.h>
// #include <I2CKeyPad.h>

// // I2C Keypad setup
// const uint8_t KEYPAD_ADDRESS = 0x20;
// I2CKeyPad keyPad(KEYPAD_ADDRESS);
// char keymap[17] = "123A456B789C*0#D";

// // Ultrasonic sensor pins
// const int trigPin = 15;  // D8
// const int echoPin = 13;   // RX
// const int maxDistance = 10; // Maximum distance in cm for cup detection
// const int valvePin = 14;  // D5 (GPIO14) for relay control

// // Add these variables at the top with other globals
// unsigned long lastKeyPress = 0;
// const unsigned long debounceTime = 300; // Increased to 300ms
// bool keyWasPressed = false;

// String keyBuffer = "";
// int state = 0;
// float waterPrice = 300;
// float totalAmount = 0;
// float waterLiters = 0;
// String phoneNumber = "";
// char inputMode = ' ';



// float readDistance() {
//   digitalWrite(trigPin, LOW);
//   delayMicroseconds(2);
//   digitalWrite(trigPin, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(trigPin, LOW);
  
//   long duration = pulseIn(echoPin, HIGH);
//   return duration * 0.034 / 2;
// }

// void setup() {
//   pinMode(valvePin, OUTPUT);
//   digitalWrite(valvePin, LOW);  // Ensure valve is closed at startup
//   Serial.begin(115200);
//   while (!Serial) { delay(10); }
  
//   Wire.begin(D2, D1);
//   Wire.setClock(100000);
  
//   pinMode(trigPin, OUTPUT);
//   pinMode(echoPin, INPUT);
  
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
//                 Serial.println("Starting water dispensing...");
//                 digitalWrite(valvePin, HIGH);
                
//                 unsigned long startTime = millis();
//                 float dispensedLiters = 0;
//                 float lastDispensedAmount = 0;
//                 const float flowRate = 0.5;
                
//                 while (dispensedLiters < waterLiters) {
//                   digitalWrite(valvePin, LOW);
//                   distance = readDistance();
                  
//                   if (distance <= maxDistance) {
//                     digitalWrite(valvePin, HIGH);
//                     unsigned long currentTime = millis();
//                     float elapsedSeconds = (currentTime - startTime) / 1000.0;
//                     dispensedLiters = lastDispensedAmount + (elapsedSeconds * flowRate);
//                     Serial.printf("Dispensed: %.2f / %.2f Liters\n", dispensedLiters, waterLiters);
//                   } else {
//                     Serial.println("Cup removed! Dispensing paused.");
//                     lastDispensedAmount = dispensedLiters;
//                     startTime = millis();
//                     while (distance > maxDistance) {
//                       distance = readDistance();
//                       delay(100);
//                     }
//                     Serial.println("Cup detected! Resuming dispensing...");
//                   }
//                   delay(100);
//                 }
                
//                 digitalWrite(valvePin, LOW);
//                 Serial.println("Dispensing completed!");
//                 Serial.println("Thank you for using our service");
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

