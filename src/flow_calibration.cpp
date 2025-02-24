// #include <Arduino.h>

// const int FLOW_SENSOR_PIN = 2;  // Flow sensor input pin
// const int RELAY_PIN = 14;       // Relay control pin
// volatile long pulseCount = 0;    // Flow sensor pulse counter
// float calibrationFactor = 7.5;   // Start with this factor and adjust based on results

// void IRAM_ATTR pulseCounter() {
//   pulseCount++;
// }

// void setup() {
//   Serial.begin(115200);
  
//   pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
//   pinMode(RELAY_PIN, OUTPUT);
//   digitalWrite(RELAY_PIN, HIGH); // Start with relay OFF
  
//   attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);
// }


// void loop() {
//   // Reset pulse counter
//   pulseCount = 0;
  
//   Serial.println("Starting 300ml dispensing test...");
//   digitalWrite(RELAY_PIN, LOW); // Turn ON relay to start flow
  
//   float milliliters = 0;
//   while (milliliters < 500) {
//     // Calculate current volume
//         milliliters = (pulseCount / calibrationFactor) * 1000 / 60; // Convert pulses to mL
    
//     Serial.print("Current volume: ");
//     Serial.print(milliliters);
//     Serial.println(" ml");
//     delay(100);
//   }
  
//   digitalWrite(RELAY_PIN, HIGH); // Turn OFF relay
//   Serial.println("Dispensing complete!");
//   Serial.print("Final pulse count: ");
//   Serial.println(pulseCount);
  
//   delay(5000); // Wait before next test
// }
