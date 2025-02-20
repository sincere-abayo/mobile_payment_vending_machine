


// #include <Arduino.h>
// #include <Keypad.h>

// // Keypad setup
// const byte n_rows = 4; 
// const byte n_cols = 4; 

// char keys[n_rows][n_cols] = {
//   {'1', '2', '3', 'A'},
//   {'4', '5', '6', 'B'},
//   {'7', '8', '9', 'C'},
//   {'*', '0', '#', 'D'}
// };

// byte rowPins[n_rows] = {16, 5, 4, 0};  
// byte colPins[n_cols] = {2, 14, 12, 13}; 

// // Ultrasonic sensor pins
// const int trigPin = 15;  // D8
// const int echoPin = 3;   // RX
// const int maxDistance = 10; // Maximum distance in cm for cup detection
// const int valvePin = 2;  // D4 pin for valve control

// Keypad myKeypad = Keypad(makeKeymap(keys), rowPins, colPins, n_rows, n_cols);

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
//   Serial.begin(115200);
//   pinMode(trigPin, OUTPUT);
//   pinMode(echoPin, INPUT);
  
//   Serial.println("\nWater Vending Machine");
//   Serial.println("Select input mode:");
//   Serial.println("A: Enter amount in RWF");
//   Serial.println("B: Enter liters of water");
// }

// void loop() {
//   char key = myKeypad.getKey();
  
//   if (key) {
//     if (key == '*') {
//       keyBuffer = "";
//       Serial.println("Input cleared");
//     } 
//     else if (key == '#') {
//       if (state == 1) {
//         float input = keyBuffer.toFloat();
//         if (input > 0) {
//           if (inputMode == 'A') {
//             totalAmount = input;
//             waterLiters = totalAmount / waterPrice;
//             Serial.print("Amount: ");
//             Serial.print(totalAmount);
//             Serial.println(" RWF");
//             Serial.print("Water: ");
//             Serial.print(waterLiters);
//             Serial.println(" Liters");
//           } else {
//             waterLiters = input;
//             totalAmount = waterLiters * waterPrice;
//             Serial.print("Water: ");
//             Serial.print(waterLiters);
//             Serial.println(" Liters");
//             Serial.print("Cost: ");
//             Serial.print(totalAmount);
//             Serial.println(" RWF");
//           }
//           Serial.println("Enter phone number:");
//           state = 2;
//         }
//         keyBuffer = "";
//       }
//       else if (state == 2) {
//         phoneNumber = keyBuffer;
//         Serial.println("Processing payment for: " + phoneNumber);
//         Serial.println("Payment completed!");
//         Serial.println("\nPLEASE PLACE YOUR CUP");
//         state = 3;
//         keyBuffer = "";
//       }
//       else if (state == 3) {
//         float distance = readDistance();
//         if (distance <= maxDistance) {
//           Serial.println("Cup detected!");
//           Serial.print("Distance: ");
//           Serial.print(distance);
//           Serial.println(" cm");
//           Serial.println("Ready for water dispensing");
//           delay(2000);
//           Serial.println("Thank you for using our service");
//           state = 0;
//           keyBuffer = "";
//           inputMode = ' ';
//           Serial.println("\nSelect input mode:");
//           Serial.println("A: Enter amount in RWF");
//           Serial.println("B: Enter liters of water");
//         } else {
//           Serial.println("No cup detected! Please place cup correctly");
//           Serial.print("Current distance: ");
//           Serial.print(distance);
//           Serial.println(" cm");
//         }
//       }
//     }
//     else if (state == 0 && (key == 'A' || key == 'B')) {
//       state = 1;
//       inputMode = key;
//       if (key == 'A') {
//         Serial.println("Enter amount in RWF:");
//       } else {
//         Serial.println("Enter liters of water:");
//       }
//     }
//     else {
//       keyBuffer += key;
//       Serial.print("Input: ");
//       Serial.println(keyBuffer);
//     }
//   }
//   delay(100);
// }
