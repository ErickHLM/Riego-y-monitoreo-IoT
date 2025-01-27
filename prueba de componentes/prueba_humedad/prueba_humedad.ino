// Define the pin connections
const int sensorPin = 34; // Analog pin where the sensor is connected
const int ledPin = 2;     // Digital pin for the onboard LED

void setup() {
    // Initialize serial communication at 115200 baud rate
    Serial.begin(115200);
    
    // Set the LED pin as an output
    pinMode(ledPin, OUTPUT);
}

void loop() {
    // Read the analog value from the sensor
    int sensorValue = analogRead(sensorPin);
    
    // Print the sensor value to the Serial Monitor
    Serial.print("Sensor Value: ");
    Serial.println(sensorValue);
    
    // Turn on the LED if the soil is dry (sensor value is high)
    if (sensorValue > 2000) {
        digitalWrite(ledPin, HIGH);
    } else {
        digitalWrite(ledPin, LOW);
    }
    
    // Wait for a second before taking another reading
    delay(1000);
}