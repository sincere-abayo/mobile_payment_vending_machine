// #include <Arduino.h>

// const int trigPin = 15;  // D8
// const int echoPin = 13;  // D7

// void setup() {
//   Serial.begin(115200);
//   pinMode(trigPin, OUTPUT);
//   pinMode(echoPin, INPUT);
  
//   Serial.println("\nUltrasonic Sensor Test");
//   Serial.println("Reading distance every 500ms");
// }

// void loop() {
//   digitalWrite(trigPin, LOW);
//   delayMicroseconds(2);
//   digitalWrite(trigPin, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(trigPin, LOW);
  
//   long duration = pulseIn(echoPin, HIGH);
//   float distance = duration * 0.034 / 2;
  
//   Serial.print("Distance: ");
//   Serial.print(distance);
//   Serial.println(" cm");
  
//   delay(500);
// }
