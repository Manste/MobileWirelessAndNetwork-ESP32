
#include "DHT.h"
#include "WiFi.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "mbedtls/aes.h"
#include "mbedtls/base64.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/sha256.h"
#include"cstring"

#define BLOCK_SIZE 16

#define RED_LED_PIN 32
#define WHITE_LED_PIN 33
#define BUTTON_PIN 27
#define DHT_PIN 26
#define DHT_TYPE DHT11     // Specify DHT11 sensor type

// Wi-Fi credentials
#define WIFI_SSID        "Nms"
#define WIFI_PASSWORD    "manououvreferme"

// MQTT Configuration
#define MQTT_SERVER      "192.168.79.56" // your MQTT server address
#define MQTT_PORT        8008 // Default MQTT port
#define MQTT_USERNAME    "esp1"                // esp2
#define MQTT_PASSWORD    "HUS49mVnyF54CT8vh889k8Aay"// 3wbjiA9bU837e6UBv5hE8AP5S

// Device-Specific Topics
#define HUMIDITY_TOPIC "esp1/humidity"
#define TEMPERATURE_TOPIC "esp1/temperature"
#define THRESHOLD_TOPIC "esp1/threshold"

// Subscribed Topics for ESP2
#define OTHER_HUMIDITY_TOPIC "esp2/humidity"
#define OTHER_THRESHOLD_TOPIC "esp2/threshold"

#define ESP1_ENCRYPT "H50Eoq18OKL+QcpnUNS4p7qwYLWBzWUZm0V+n85mjLI="
#define ESP1_HASH "LEaiLUukPRTGtPQWhMuxdcVwEjgrcxBG"

#define ESP2_ENCRYPT "t6PeycMhrW003pbO2W8YPI54jt18BZmqIfwpxHgZbyQ="
#define ESP2_HASH "JAYWBfudauenBjnPUENgnEeYXmLnwSjN"

#define SERVER_ENCRYPT "fTmJ6IfIHA+ebpztbkePGgZ7J2Dxd1xxDF9aHSjb0FI="
#define SERVER_HASH "SzMVxXdSkmvmxmFGHrUJQspxHvCxKqFW"

unsigned char* esp1Key = nullptr;
unsigned char* esp2Key = nullptr;
unsigned char* serverKey = nullptr;

// states
bool lastButtonState = HIGH;
bool isSubscribed = false;
float currentOtherThreshold = 50.0;
float currentOtherHumidity = 0.0;
bool isHandshaked = false;

mbedtls_entropy_context entropy;

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
      // Re-subscribe to topics
      mqtt.subscribe(&other_humidity_subscriber);
      mqtt.subscribe(&other_threshold_subscriber);
    } else {
      Serial.print("Failed to connect to MQTT broker, error: ");
      Serial.println(mqtt.connectErrorString(ret));
      Serial.println("Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// Pad input to a fixed size using PKCS#7 padding
void padInput(String& input, char* paddedInput) {
    size_t inputLength = input.length();
    size_t paddingLength = BLOCK_SIZE - (inputLength % BLOCK_SIZE);
    memcpy(paddedInput, input.c_str(), inputLength); // Copy the input data
    for (size_t i = inputLength; i < BLOCK_SIZE; i++) {
        paddedInput[i] = (char) paddingLength;
    }
    paddedInput[BLOCK_SIZE] = '\0'; // Null-terminate the padded input
}

// Remove padding from decrypted data
size_t unpadOutput(const unsigned char *decrypted, size_t decrypted_len, unsigned char *output) {
    if (decrypted_len == 0) {
        return 0;
    }
    size_t padding_len = decrypted[decrypted_len - 1];
    size_t original_len = decrypted_len - padding_len;
    memcpy(output, decrypted, original_len);  // Copy unpadded data to output
    output[original_len] = '\0';
    return original_len;
}

void preparePayload(String data, unsigned char* key, const char* hashKey, Adafruit_MQTT_Publish &publisher) {
  mbedtls_ctr_drbg_context ctr_drbg;
  char *personalization = MQTT_USERNAME;
  mbedtls_ctr_drbg_init(&ctr_drbg);
  int error = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,(const unsigned char *) personalization, strlen(personalization) );
  if(error == 0){ 
    unsigned char iv[BLOCK_SIZE];
    unsigned char ivcp[BLOCK_SIZE];
    // generate the input value
    mbedtls_ctr_drbg_random(&ctr_drbg, iv, BLOCK_SIZE);
    memcpy(ivcp, iv, BLOCK_SIZE);
    
    char paddedData[BLOCK_SIZE];
    padInput(data, paddedData);
    
    // encryption
    unsigned char* cipherText = (unsigned char*)malloc(strlen(paddedData) + 1);
    cipherText[strlen((const char*) paddedData)] = '\0';
    mbedtls_aes_context aes;
    mbedtls_aes_setkey_enc(&aes, key, 256 );
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, strlen((const char*) paddedData), iv, (const unsigned char*) paddedData, cipherText);
    mbedtls_aes_free(&aes);
    
    // iv, hash and cipherText to base64
    size_t written = 0, outsize = 0;
    mbedtls_base64_encode (NULL, 0, &written, ivcp, BLOCK_SIZE);
    unsigned char* ivB64 = (unsigned char*)malloc(sizeof(char)*(written+1));
    mbedtls_base64_encode (ivB64, written, &outsize, ivcp, BLOCK_SIZE);
    ivB64[written] = '\0';
    
    mbedtls_base64_encode (NULL, 0, &written, cipherText, strlen((const char*) paddedData));
    unsigned char* cipherTextB64 = (unsigned char*)malloc(sizeof(char)*(written+1));
    mbedtls_base64_encode (cipherTextB64, written, &outsize, cipherText, strlen((const char*) paddedData));
    cipherTextB64[written] = '\0';   
    
    //hashing
    unsigned char hash[33];
    //computeHash(cipherTextB64cp, hash, strlen((const char*) cipherTextB64cp), hashKey);
    char dataToHash[strlen((char*)cipherTextB64) + 2 + strlen(hashKey)];
    strcpy(dataToHash,(const char*) cipherTextB64);
    strcat(dataToHash,".");
    strcat(dataToHash,hashKey);
    mbedtls_sha256((const unsigned char *) dataToHash, strlen((const char*) dataToHash), hash, 0);
    hash[32]='\0';
    
    mbedtls_base64_encode (NULL, 0, &written, hash, 32);
    unsigned char* hashB64 = (unsigned char*)malloc(sizeof(char)*(written+1));
    mbedtls_base64_encode (hashB64, written, &outsize, hash, 32);
    hashB64[written] = '\0';
    
    size_t totalLength = strlen((const char*)ivB64) + strlen((const char*)hashB64) + strlen((const char*)cipherTextB64) + 3;
    char* result = (char*)malloc(totalLength);
    strcpy(result,(const char*) cipherTextB64);
    strcat(result,".");
    strcat(result,(const char*) hashB64);
    strcat(result,".");
    strcat(result,(const char*) ivB64);

    publishData(publisher, result);
    free(cipherText);
    free(cipherTextB64);
    free(hashB64);
    free(ivB64);
    mbedtls_ctr_drbg_free(&ctr_drbg);
  }
  mbedtls_ctr_drbg_free(&ctr_drbg);
}

void publishData(Adafruit_MQTT_Publish &publisher, char* result) {
   if (!publisher.publish(result)) {
    Serial.print("Failed to publish data!");
    Serial.println(result);
  } else {
    Serial.print("Sensor published: ");
    Serial.println(result);
  }
}

// Process the received payload from `publishData`
char* processReceivedPayload(char* p, unsigned char* key, const char* hashKey) {
    String payload(p);
    size_t written = 0, outsize = 0, cipherTextLength = 0;
    int delimiterPos1 = payload.indexOf('.');
    int delimiterPos2 = payload.lastIndexOf('.');

     if (delimiterPos1 == -1 || delimiterPos2 <= delimiterPos1) {
        return "Error: Invalid payload format.";
    }
    String cipherTextB64 = payload.substring(0, delimiterPos1);
    String hashB64 = payload.substring(delimiterPos1 + 1, delimiterPos2);
    String ivB64 = payload.substring(delimiterPos2 + 1);
    
    // The cyphered message
    mbedtls_base64_decode(NULL, 0, &written, (const unsigned char *) cipherTextB64.c_str(), cipherTextB64.length());
    unsigned char* cipherText = (unsigned char*)malloc(sizeof(char)*(written+1));
    mbedtls_base64_decode(cipherText, written, &cipherTextLength, (const unsigned char *) cipherTextB64.c_str(), cipherTextB64.length());
    cipherText[written] = 0;
    
    // The iv
    mbedtls_base64_decode(NULL, 0, &written, (const unsigned char *) ivB64.c_str(), ivB64.length());
    unsigned char* iv = (unsigned char*)malloc(sizeof(char)*(written+1));
    mbedtls_base64_decode(iv, written, &outsize, (const unsigned char *) ivB64.c_str(), ivB64.length());
    iv[written] = 0;

    // verify hash
    unsigned char computedHash[33];
    char dataToHash[cipherTextB64.length() + 2 + strlen(hashKey)];
    strcpy(dataToHash,(const char*) cipherTextB64.c_str());
    strcat(dataToHash,".");
    strcat(dataToHash,hashKey);
    Serial.println(dataToHash);
    mbedtls_sha256((const unsigned char *) dataToHash, strlen((const char*) dataToHash), computedHash, 0);
    computedHash[32] = '\0';
    
    mbedtls_base64_encode (NULL, 0, &written, computedHash, strlen((const char*) computedHash));
    unsigned char* computedHashB64 = (unsigned char*)malloc(sizeof(char)*(written+1));
    mbedtls_base64_encode (computedHashB64, written, &outsize, computedHash, strlen((const char*) computedHash));
    computedHashB64[written] = '\0';  

    //Serial.println((char*)computedHashB64);
    //Serial.println((char*)hashB64.c_str());
    if(strcmp((const char*) computedHashB64, hashB64.c_str()) == 0){
      unsigned char* message = (unsigned char*)malloc(sizeof(char)*(cipherTextLength+1));
      message[cipherTextLength] = 0;
      mbedtls_aes_context aes;
      mbedtls_aes_setkey_dec(&aes, key, 256 );
      mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, cipherTextLength, iv, cipherText, (unsigned char *)message);
      mbedtls_aes_free(&aes);
      unsigned char* result = (unsigned char*)malloc(cipherTextLength + 1);
      unpadOutput(message, cipherTextLength, result);
      free(message);
      free(cipherText);
      free(computedHashB64);
      free(iv);
      return (char*)result;
    }
    free(cipherText);
    free(computedHashB64);
    free(iv);
    return "";
}

void updateRedLed() {
  if (isSubscribed) {
    if (currentOtherHumidity >= currentOtherThreshold) {
      digitalWrite(RED_LED_PIN, HIGH);
    } else {
      digitalWrite(RED_LED_PIN, LOW);
    }
  } else {
    digitalWrite(RED_LED_PIN, LOW);
  }
}

void manageButton() {
   bool buttonState = digitalRead(BUTTON_PIN); // Read the button state

  // Check if the button state has changed
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) { // Check if the button is pressed (LOW because of pull-up)
      isSubscribed = !isSubscribed; // Toggle LED state
      if (isSubscribed) {
        // Subscribe to topics
        digitalWrite(WHITE_LED_PIN, HIGH); // Update LED
        Serial.println("Subscribed!");
      } else {
        digitalWrite(WHITE_LED_PIN, LOW); // Update LED
        Serial.println("Unsubscribed!");
      }
      updateRedLed();
    }
    lastButtonState = buttonState; // Save the current button state
  }
}

void humidityCallback(char* payload, uint16_t len) {
  if (isSubscribed) {
    payload[len] = '\0';  // Null-terminate payload
    Serial.println(payload);
    char* data = processReceivedPayload(payload, esp2Key, ESP2_HASH);
    if (data == "") {
      return;
    }
    currentOtherHumidity = atof(data); // Parse as float
    Serial.print("Received humidity from other ESP: ");
    Serial.println(currentOtherHumidity);
    updateRedLed();
  }
}

void thresholdCallback(char* payload, uint16_t len) {
  if (isSubscribed) {
    payload[len] = '\0';  // Null-terminate payload
    char* data = processReceivedPayload(payload, serverKey, SERVER_HASH);
    if (data == "") {
      return;
    }
    currentOtherThreshold = atof(data); // Parse as float
    Serial.print("Received threshold from other ESP: ");
    Serial.println(currentOtherThreshold);
    updateRedLed();
  }
}

void initializeKeys(const char* keyB64, unsigned char** key) {
    size_t written = 0, outsize = 0;
    mbedtls_base64_decode(NULL, 0, &written, (const unsigned char*)keyB64, strlen(keyB64));
    *key = (unsigned char*)malloc(sizeof(char)*(written+1));
    mbedtls_base64_decode(*key, written, &outsize, (const unsigned char*)keyB64, strlen(keyB64));
}

void setup() {
  Serial.begin(9600); // Initialize serial communication

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(WHITE_LED_PIN, OUTPUT); // Set LED pin as output
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set button pin as input with internal pull-up
  digitalWrite(RED_LED_PIN, LOW); // Start with the LED off
  digitalWrite(WHITE_LED_PIN, LOW); // Start with the LED off

  mbedtls_entropy_init(&entropy);

  dht.begin(); // Start the DHT sensor
  setup_wifi();

  // Set callback functions
  other_humidity_subscriber.setCallback(humidityCallback);
  other_threshold_subscriber.setCallback(thresholdCallback);

  mqtt.connect();
  mqtt.subscribe(&other_humidity_subscriber);
  mqtt.subscribe(&other_threshold_subscriber);

  while (!esp1Key || !esp2Key || !serverKey) {    
      initializeKeys(ESP1_ENCRYPT, &esp1Key);
      initializeKeys(ESP2_ENCRYPT, &esp2Key);
      initializeKeys(SERVER_ENCRYPT, &serverKey);
  }
  
}

void loop() {
  // Ensure the MQTT connection
  if (!mqtt.connected()) {
    reconnect();
  }

  mqtt.processPackets(2000); // Process incoming messages

  manageButton();

  // Temperature and Humidity Reading
  float humidity = dht.readHumidity(); // Read humidity
  float temperature = dht.readTemperature(); // Read temperature in Celsius

  // Check if any reads failed and retry
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    //preparePayload(String(humidity), esp1Key, ESP1_HASH, humidity_publisher);
    //preparePayload(String(temperature), esp1Key, ESP1_HASH, temperature_publisher); 
    char* tmp = processReceivedPayload("Dqslq6dIB+4QiOmwZUmsOQ==.RNefAeCA3grfkwITl0/oK40o6mckH09DH/hHFekK8iQ=.U217G40cBIxzoxeJ3mELWg==", serverKey, SERVER_ENCRYPT);
    Serial.println(tmp);
  }
  delay(1000);
}
