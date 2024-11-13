#include "DHT.h"

#define RED_LED_PIN 32
#define WHITE_LED_PIN 33
#define BUTTON_PIN 27
#define DHT_PIN 26
#define DHT_TYPE DHT11     // Specify DHT11 sensor type

DHT dht(DHT_PIN, DHT_TYPE); // Initialize DHT sensor

bool redLedState = false; // Variable to store LED state
bool whiteLedState = false; // Variable to store LED state
bool lastButtonState = false;// Variable to store last button state of the red led

void setup() {
  Serial.begin(9600); // Initialize serial communication
  
  pinMode(RED_LED_PIN, OUTPUT); 
  pinMode(WHITE_LED_PIN, OUTPUT); // Set LED pin as output
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set button pin as input with internal pull-up
  digitalWrite(RED_LED_PIN, LOW); // Start with the LED off
  digitalWrite(WHITE_LED_PIN, LOW); // Start with the LED off
  
  dht.begin(); // Start the DHT sensor
}

void loop() {
  buttonState = digitalRead(BUTTON_PIN); // Read the button state

  // Check if the button state has changed
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) { // Check if the button is pressed (LOW because of pull-up)
      redLedState = !redLedState; // Toggle LED state
      whiteLedState = redLedState; // Toggle LED state
      digitalWrite(RED_LED_PIN, redLedState ? HIGH : LOW); // Update LED
      digitalWrite(WHITE_LED_PIN, !redLedState ? HIGH : LOW); // Update LED
    }    
    lastButtonState = buttonState; // Save the current button state
  }

  // Temperature and Humidity Reading
  float humidity = dht.readHumidity(); // Read humidity
  float temperature = dht.readTemperature(); // Read temperature in Celsius
  
  // Check if any reads failed and retry
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    // Print temperature and humidity to Serial Monitor
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C");
  }
}
