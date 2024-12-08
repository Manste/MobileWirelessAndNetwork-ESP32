#include "DHT.h"
#include "WiFi.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "mbedtls/aes.h"
#include "mbedtls/md.h"
#include "mbedtls/sha256.h"
#include "mbedtls/cipher.h"

#define BLOCK_SIZE 16
#define OUTPUT_SIZE 64

#define RED_LED_PIN 32
#define WHITE_LED_PIN 33
#define BUTTON_PIN 27
#define DHT_PIN 26
#define DHT_TYPE DHT11     // Specify DHT11 sensor type

// Wi-Fi credentials
#define WIFI_SSID        "Nms"
#define WIFI_PASSWORD    "manououvreferme"

// MQTT Configuration
#define MQTT_SERVER      "192.168.76.56" // your MQTT server address
#define MQTT_PORT        8008 // Default MQTT port 
#define MQTT_USERNAME    "esp2" //"esp1"                // esp2 
#define MQTT_PASSWORD    "3wbjiA9bU837e6UBv5hE8AP5S" //"HUS49mVnyF54CT8vh889k8Aay"// 3wbjiA9bU837e6UBv5hE8AP5S 

// Device-Specific Topics
#define HUMIDITY_TOPIC "esp1/humidity"
#define TEMPERATURE_TOPIC "esp1/temperature"
#define THRESHOLD_TOPIC "esp1/threshold"

// Subscribed Topics for ESP2
#define OTHER_HUMIDITY_TOPIC "esp2/humidity"
#define OTHER_THRESHOLD_TOPIC "esp2/threshold"

#define ESP1_ENCRYPT "CmvgVtFuSNmQMCaRmLzzrCaPBFphSuYr"
#define ESP1_HASH "LEaiLUukPRTGtPQWhMuxdcVwEjgrcxBG"

#define ESP2_ENCRYPT "dgCGZGDCRwEVSczHrttbcpMALwweVxnP"
#define ESP2_HASH "JAYWBfudauenBjnPUENgnEeYXmLnwSjN"

#define SERVER_ENCRYPT "xSjxNewFTELfqmDdbgAWrDkTfNAfNYNs"
#define SERVER_HASH "SzMVxXdSkmvmxmFGHrUJQspxHvCxKqFW"

// AES and HMAC Keys
const char* aes_key = ESP2_ENCRYPT;
const char* hmac_key = ESP2_HASH;

// states
bool lastButtonState = HIGH;
bool isSubscribed = false;
float currentOtherThreshold = 50.0;
float currentOtherHumidity = 0.0;

// Nonce (Incrementing Counter)
uint32_t nonce = 0;

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
      if(isSubscribed) {
        // Re-subscribe to topics
        mqtt.subscribe(&other_humidity_subscriber);
        mqtt.subscribe(&other_threshold_subscriber);
      }
    } else {
      Serial.print("Failed to connect to MQTT broker, error: ");
      Serial.println(mqtt.connectErrorString(ret));
      Serial.println("Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// Generate a random initial nonce at startup
void initializeNonce() {
  nonce = random(1, 0xFFFFFF); // Generate a random 24-bit integer
}

void incrementNonce() {
  nonce++;  // Increment the nonce
}

// Function to apply PKCS7 padding
void addPadding(uint8_t* data, size_t& len, size_t block_size = BLOCK_SIZE) {
    size_t padding_needed = block_size - (len % block_size);
    for (size_t i = 0; i < padding_needed; i++) {
        data[len + i] = padding_needed;
    }
    len += padding_needed;
}

// Function to remove PKCS7 padding
void removePadding(uint8_t* data, size_t len) {
    size_t padding_length = data[len - 1];
    len -= padding_length;
}

String byteArrayToHex(const uint8_t* byteArray, size_t len) {
    String hexString = "";
    for (size_t i = 0; i < len; i++) {
        hexString += String(byteArray[i], HEX);;
    }
    return hexString;
}

// HMAC-SHA256
String computeHMAC(const uint8_t *key, byte* iv, String& str) {    
    // Prepare the input data with padding
    uint8_t input[OUTPUT_SIZE] = {0};
    str.toCharArray((char*)input, sizeof(input));
    
    unsigned char hmac[32]; // SHA-256 output size
    //The code comes from: https://github.com/Mbed-TLS/mbedtls/blob/development/programs/hash/md_hmac_demo.c
     const mbedtls_md_type_t alg = MBEDTLS_MD_SHA256;
     const mbedtls_md_info_t *info = mbedtls_md_info_from_type(alg);
     
    /* prepare context and load key */
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, info, 1); // the last argument to setup is 1 to enable HMAC (not just hashing)
    /* compute HMAC*/
    mbedtls_md_hmac_starts(&ctx, key, sizeof(key));
    mbedtls_md_hmac_update(&ctx, (const unsigned char*) iv, sizeof(iv));
    mbedtls_md_hmac_update(&ctx, (const unsigned char*) input, sizeof(input));
    mbedtls_md_hmac_finish(&ctx, hmac);
    mbedtls_md_free(&ctx);

    // Convert HMAC to hex string
    String hmac_str = byteArrayToHex(hmac, sizeof(hmac));
  
    return hmac_str;    
}

bool verifyHMAC(const uint8_t* key, byte* iv, String& encrypted_data, String& received_hmac) {
    String computed_hmac = computeHMAC(key, iv, encrypted_data);

    // Perform a case-insensitive comparison of HMAC strings
    return computed_hmac.equalsIgnoreCase(received_hmac);
}


// AES-256-CBC Encryption
String aesEncrypt(const uint8_t* key, byte* iv, const String& str) {
    size_t input_len = str.length();
    size_t padded_len = input_len;

    // Prepare the input data with padding
    uint8_t input[OUTPUT_SIZE] = {0};
    str.toCharArray((char*)input, sizeof(input));
    addPadding(input, padded_len);

    // Prepare output buffer
    uint8_t encrypted[OUTPUT_SIZE] = {0}; // Ensure it matches the padded input size

    // Initialize AES context
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    // Set AES encryption key
    if (mbedtls_aes_setkey_enc(&aes, key, 256) != 0) {
        mbedtls_aes_free(&aes);
        return "Error: Failed to set AES encryption key";
    }
    // Perform AES-CBC encryption
    if (mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, padded_len, iv, input, encrypted) != 0) {
        mbedtls_aes_free(&aes);
        return "Error: AES encryption failed";
    }
    // Free the AES context
    mbedtls_aes_free(&aes);

    // Convert encrypted data to a hexadecimal string
    String encrypted_data = byteArrayToHex(encrypted, sizeof(encrypted));
    return encrypted_data;
}

// AES-256-CBC Decryption
String aesDecrypt(const uint8_t *key, byte* iv, uint8_t* input, size_t input_len) { 
    size_t len = sizeof(input);
    uint8_t output[OUTPUT_SIZE] = {0};
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    
    // Set AES decryption key
    if (mbedtls_aes_setkey_dec(&aes, key, 256) != 0) {
        mbedtls_aes_free(&aes);
        return "Error: Failed to set AES decryption key";
    }
    // Perform AES-CBC decryption
    if (mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, input_len, iv, input, output) != 0) {
        mbedtls_aes_free(&aes);
        return "Error: AES decryption failed";
    }
    // Free the AES context
    mbedtls_aes_free(&aes);

    
    // Remove padding
    removePadding(output, len);

    // Convert decrypted byte array to string
    String decrypted_data = "";
    for (size_t i = 0; i < len; i++) {
        decrypted_data += (char)output[i];
    }

    return decrypted_data;
}

String processPayload(const String& payload) {
    // Step 1: Split payload
    int iv_end = payload.indexOf('+');
    int data_end = payload.lastIndexOf('+');

    if (iv_end == -1 || data_end == -1 || data_end <= iv_end) {
        Serial.println("Error: Invalid payload format.");
        return "Error: Invalid payload format.";
    }

    // Extract components
    String iv_hex = payload.substring(0, iv_end);
    String encrypted_data = payload.substring(iv_end + 1, data_end);
    String received_hmac = payload.substring(data_end + 1);

    // Step 2: Convert IV from hex to byte array
    uint8_t iv[BLOCK_SIZE] = {0};
    for (size_t i = 0; i < iv_hex.length() / 2; i++) {
        iv[i] = strtol(iv_hex.substring(i * 2, i * 2 + 2).c_str(), nullptr, 16);
    }

    // Step 3: Verify HMAC
    if (!verifyHMAC((uint8_t*)hmac_key, iv, encrypted_data, received_hmac)) {
        Serial.println("HMAC verification failed!");
        return "HMAC verification failed!";
    }

    // Step 4: Decrypt the data
    size_t len = encrypted_data.length() / 2; // Convert hex to byte length
    uint8_t encrypted_bytes[OUTPUT_SIZE] = {0};

    // Convert encrypted hex string to byte array
    for (size_t i = 0; i < len; i++) {
        encrypted_bytes[i] = strtol(encrypted_data.substring(i * 2, i * 2 + 2).c_str(), nullptr, 16);
    }

    // Perform decryption
    String decrypted_data = aesDecrypt((uint8_t*)aes_key, iv, encrypted_bytes, sizeof(encrypted_bytes));   // Step 6: Store or log the decrypted data
    return decrypted_data;
}


void publishData(Adafruit_MQTT_Publish &publisher, String s) {
  byte iv[BLOCK_SIZE] = {0}; // Initialization Vector
  memcpy(iv, &nonce, sizeof(nonce)); // Use the nonce as part of the IV
  String payload = byteArrayToHex(iv, sizeof(iv)); 
  
  String encrypted_data = aesDecrypt(aes_key, iv, (const char*)s, s.length());

  if (encrypted_data.length()) {
    String hmac = computeHMAC((uint8_t *)hmac_key, iv, encrypted_data);

    // Transmit: IV + Encrypted Data + HMAC
    String payload = payload + "+" + encrypted_data + "+" + hmac;
    
    if (!publisher.publish(payload.c_str())) {
      Serial.println("Failed to publish data!");
    } else {    
      Serial.println("Sensor published: " + payload);
    }
  }
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
        // subscribe to topics
        mqtt.subscribe(&other_humidity_subscriber);
        mqtt.subscribe(&other_threshold_subscriber);        
        digitalWrite(WHITE_LED_PIN, HIGH); // Update LED
        Serial.println("Subscribed!");
      } else {        
        mqtt.unsubscribe(&other_humidity_subscriber);
        mqtt.unsubscribe(&other_threshold_subscriber);        
        digitalWrite(WHITE_LED_PIN, HIGH); // Update LED
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
    currentOtherHumidity = atof(payload); // Parse as float
    Serial.print("Received humidity from other ESP: ");
    Serial.println(currentOtherHumidity);
    updateRedLed();
  }
}

void thresholdCallback(char* payload, uint16_t len) {
  if (isSubscribed) {
    payload[len] = '\0';  // Null-terminate payload
    currentOtherThreshold = atof(payload); // Parse as float
    Serial.print("Received threshold from other ESP: ");
    Serial.println(currentOtherThreshold);
    updateRedLed();
  }
}

void setup() {
  Serial.begin(9600); // Initialize serial communication
  
  randomSeed(analogRead(0)); // Seed the random generator
  
  pinMode(RED_LED_PIN, OUTPUT); 
  pinMode(WHITE_LED_PIN, OUTPUT); // Set LED pin as output
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set button pin as input with internal pull-up
  digitalWrite(RED_LED_PIN, LOW); // Start with the LED off
  digitalWrite(WHITE_LED_PIN, LOW); // Start with the LED off
  
  dht.begin(); // Start the DHT sensor
  setup_wifi();

  // Set callback functions
  other_humidity_subscriber.setCallback(humidityCallback);
  other_threshold_subscriber.setCallback(thresholdCallback);
  
  mqtt.connect();
  initializeNonce();
}

void loop() {
  // Ensure the MQTT connection
  if (!mqtt.connected()) {
    reconnect();
  }

  mqtt.processPackets(2000); // Process incoming messages

  manageButton();
  incrementNonce();
  
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
  delay(1000);
}
