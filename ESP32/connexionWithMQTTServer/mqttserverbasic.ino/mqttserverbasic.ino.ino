#include "DHT.h"
#include "WiFi.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define RED_LED_PIN 32
#define WHITE_LED_PIN 33
#define BUTTON_PIN 27
#define DHT_PIN 26
#define DHT_TYPE DHT11     // Specify DHT11 sensor type

// WiFi credentials
#define WIFI_SSID        "Nms"
#define WIFI_PASSWORD    "manououvreferme"
#define MQTT_SERVER      "192.168.118.56" // your MQTT server address
#define MQTT_PORT        8008 // Default MQTT port
#define MQTT_USERNAME    ""                // Leave blank for anonymous vGlrsiXas6m6rB1EeP1F
#define MQTT_PASSWORD    ""                // Leave blank for anonymous

// LED states
bool redLedState = false; // Variable to store LED state
bool whiteLedState = false; // Variable to store LED state
bool lastButtonState = false;// Variable to store last button state of the red led
bool buttonState = false;


DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient wifi_client;
Adafruit_MQTT_Client mqtt(&wifi_client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
Adafruit_MQTT_Publish data_publisher = Adafruit_MQTT_Publish(&mqtt, "esp1/sensorData");

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT broker...");
    int8_t ret = mqtt.connect();
    if (ret == 0) {
      Serial.println("connected!");
    } else {
      Serial.print("Failed to connect to MQTT broker, error: ");
      Serial.println(mqtt.connectErrorString(ret));
      Serial.println("Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600); // Initialize serial communication
  
  pinMode(RED_LED_PIN, OUTPUT); 
  pinMode(WHITE_LED_PIN, OUTPUT); // Set LED pin as output
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set button pin as input with internal pull-up
  digitalWrite(RED_LED_PIN, LOW); // Start with the LED off
  digitalWrite(WHITE_LED_PIN, LOW); // Start with the LED off
  
  dht.begin(); // Start the DHT sensor
  setup_wifi();
  reconnect();
}

void loop() {
  // Ensure the MQTT connection
  if (!mqtt.connected()) {
    reconnect();
  }
  
  // Temperature and Humidity Reading
  float humidity = dht.readHumidity(); // Read humidity
  float temperature = dht.readTemperature(); // Read temperature in Celsius
  
  // Check if any reads failed and retry
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    // Create a JSON payload
    String json_payload = "{";
    json_payload += "\"temperature\":";
    json_payload += String(temperature, 2); // Include 2 decimal places
    json_payload += ",\"humidity\":";
    json_payload += String(humidity, 2);
    json_payload += "}";
  
    // Publish JSON payload
    if (!data_publisher.publish(json_payload.c_str())) {
      Serial.println("Failed to publish sensor data!");
    } else {
      Serial.println("Sensor data published: " + json_payload);
    }
  }

  // Button Handling - Toggle LED states when pressed
  buttonState = digitalRead(BUTTON_PIN) == LOW;  // Button is pressed when LOW

  // If the button state has changed from last state, toggle LEDs
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      // Toggle LED states when button is pressed
      redLedState = !redLedState;
      whiteLedState = redLedState;
      
      // Update LEDs
      digitalWrite(RED_LED_PIN, redLedState ? HIGH : LOW);
      digitalWrite(WHITE_LED_PIN, whiteLedState ? HIGH : LOW);
    }
    // Update last button state
    lastButtonState = buttonState;
  }
  delay(2000); // Wait before sending the next update
}
