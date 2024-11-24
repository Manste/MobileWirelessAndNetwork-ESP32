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

// MQTT Configuration
#define MQTT_SERVER      "192.168.64.56" // your MQTT server address
#define MQTT_PORT        8008 // Default MQTT port
#define MQTT_USERNAME    ""                // Leave blank for anonymous vGlrsiXas6m6rB1EeP1F
#define MQTT_PASSWORD    ""                // Leave blank for anonymous

// Device-Specific Topics
#define HUMIDITY_TOPIC "esp1/humidity"
#define TEMPERATURE_TOPIC "esp1/temperature"
#define THRESHOLD_TOPIC "esp1/threshold"

// Subscribed Topics for ESP2
#define OTHER_HUMIDITY_TOPIC "esp1/humidity"
#define OTHER_THRESHOLD_TOPIC "esp1/threshold"

// states
bool lastButtonState = HIGH;
bool isSubscribed = false;
float currentOtherThreshold = 50.0;
float currentOtherHumidity = 0.0;

// MQTT Client
DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient wifi_client;
Adafruit_MQTT_Client mqtt(&wifi_client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
Adafruit_MQTT_Publish humidity_publisher = Adafruit_MQTT_Publish(&mqtt, HUMIDITY_TOPIC);
Adafruit_MQTT_Publish temperature_publisher = Adafruit_MQTT_Publish(&mqtt, TEMPERATURE_TOPIC);

Adafruit_MQTT_Subscribe other_humidity_subscriber = Adafruit_MQTT_Subscribe(&mqtt, OTHER_HUMIDITY_TOPIC);
Adafruit_MQTT_Subscribe other_threshold_subscriber = Adafruit_MQTT_Subscribe(&mqtt, OTHER_THRESHOLD_TOPIC);

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

// Unsubscribe from Other Topics
void unsubscribeFromOtherTopics() {
  if (isSubscribed) {
    Serial.println("Unsubscribing from other topics...");
    mqtt.unsubscribe(&other_humidity_subscriber);
    mqtt.unsubscribe(&other_threshold_subscriber);
    
    currentOtherThreshold = 50.0;
    currentOtherHumidity = 0.0;
    digitalWrite(WHITE_LED_PIN, LOW); // Turn off LED to indicate unsubscription
    digitalWrite(RED_LED_PIN, LOW);
    isSubscribed = false;
  }
}

// Subscribe to Other Topics
void subscribeToOtherTopics() {
  if (!isSubscribed) {
    Serial.println("Subscribing to other topics...");
    mqtt.subscribe(&other_humidity_subscriber);
    mqtt.subscribe(&other_threshold_subscriber);
    digitalWrite(WHITE_LED_PIN, HIGH); // Indicate subscription with LED
    isSubscribed = true;
  }
}

void publishData (Adafruit_MQTT_Publish &publisher, String s) {
  if (!publisher.publish(s.c_str())) {
    Serial.println("Failed to publish data!");
  } else {
    Serial.println("Sensor published: " + s);
  }
}

// Button Management - Toggle Subscription
void manageButton() {
  // Button Handling - Toggle LED states when pressed
  bool buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW && lastButtonState == HIGH) { // Button press detected
    if (isSubscribed) {
      unsubscribeFromOtherTopics();
    } else {
      subscribeToOtherTopics();
    }
  }
  lastButtonState = buttonState;
}

void updateRedLed() {
  if (isSubscribed) {
    if (currentOtherHumidity >= currentOtherThreshold) {
      digitalWrite(RED_LED_PIN, HIGH);
    } else {
      digitalWrite(RED_LED_PIN, LOW);
    }
  }
}

// Handle MQTT Messages from subscriptions
void handleMQTTSubscribtions() {
   if (isSubscribed) {
      Adafruit_MQTT_Subscribe *subscription;
      while ((subscription = mqtt.readSubscription(1000))) {
        if (subscription == &other_humidity_subscriber) {
          currentOtherThreshold = atof((char *)other_humidity_subscriber.lastread);
          Serial.print("Received humidity from other ESP: ");
          Serial.println(currentOtherThreshold);
          
        } else if (subscription == &other_threshold_subscriber) {
          currentOtherThreshold = atoi((char *)other_threshold_subscriber.lastread);
          Serial.print("Received threshold from other ESP: ");
          Serial.println(currentOtherThreshold);
          
        }
      }
   }
  updateRedLed();
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
   
}


void loop() {
  // Ensure the MQTT connection
  if (!mqtt.connected()) {
    reconnect();
  }

  // function for increasing the threshold
  manageButton();
  
  // Temperature and Humidity Reading
  float humidity = dht.readHumidity(); // Read humidity
  float temperature = dht.readTemperature(); // Read temperature in Celsius
  
  // Check if any reads failed and retry
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {  
    publishData(humidity_publisher, String(humidity));
    publishData(temperature_publisher, String(temperature));
  }

  handleMQTTSubscribtions();
  
  delay(1000); // Wait before sending the next update
}
