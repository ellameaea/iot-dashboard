#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

// --- WiFi Credentials ---
const char* ssid = "MRSP WIFI";         // Replace with your WiFi Name
const char* password = "MRSP@NUMOA2026"; // Replace with your WiFi Password

// --- HiveMQTT Credentials ---
const char* mqtt_server = "92cb9acce1604fac9799f6d34d8770d6.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;  
const char* mqtt_user = "ellamae";
const char* mqtt_password = "Ellamae2004";

// --- Pin Definitions ---
#define DHTPIN 16         // Standard digital pin for DHT11
#define DHTTYPE DHT11     // Specify DHT 11 

#define PIRPIN 17         // Standard digital pin for PIR sensor
#define MQ2PIN 34         // Input-Only analog pin for MQ2
#define MOISTUREPIN 36    // Input-Only analog pin for Moisture sensor

// --- Threshold Definitions ---
// Note: Adjust this value! Most sensors read HIGH (e.g., 4000+) in air 
// and LOW (e.g., < 1500) in water. Test your specific sensor to find the sweet spot.
const int WATER_THRESHOLD = 4000; 

// --- Initialize Objects ---
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Timer variable to replace delay()
unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected to the broker
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0, 0xffff), HEX);
    
    // Attempt to connect with User and Password
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected to HiveMQ!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Sensor Initialization...");

  // Initialize Sensors
  dht.begin();
  pinMode(PIRPIN, INPUT);
  pinMode(MQ2PIN, INPUT);
  pinMode(MOISTUREPIN, INPUT);
  
  // Setup WiFi and MQTT
  setup_wifi();
  
  // Important: Skip certificate validation for quick setup on port 8883
  espClient.setInsecure(); 
  
  client.setServer(mqtt_server, mqtt_port);

  // Allow sensors a moment to warm up
  delay(2000); 
}

void loop() {
  // Ensure we stay connected to MQTT
  if (!client.connected()) {
    reconnect();
  }
  // Allow the MQTT client to process incoming messages and maintain its connection
  client.loop();

  // Non-blocking timer: Read sensors every 2 seconds (2000 ms)
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    // --- 1. Read and Publish DHT11 Sensor ---
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature(); 

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
      Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
      
      // Publish to HiveMQ
      client.publish("sensors/temperature", String(temperature).c_str());
     
    }

    // --- 2. Read and Publish MQ2 Gas Sensor ---
    int mq2Value = analogRead(MQ2PIN);
    Serial.print("MQ2 Gas Value: "); Serial.println(mq2Value);
    client.publish("sensors/gas", String(mq2Value).c_str());

    // --- 3. Read and Publish PIR Motion Sensor ---
    int pirState = digitalRead(PIRPIN);
    Serial.print("Motion Detected: ");
    if (pirState == HIGH) {
      Serial.println("YES");
      client.publish("sensors/pir", "1"); // Sending 1 for motion
    } else {
      Serial.println("NO");
      client.publish("sensors/pir", "0"); // Sending 0 for no motion
    }

    // --- 4. Read and Publish Soil Moisture Sensor ---
    int moistureValue = analogRead(MOISTUREPIN);
    Serial.print("Soil Moisture Value: "); Serial.print(moistureValue);
    
    // Evaluate if the sensor is submerged based on the threshold
    if (moistureValue < WATER_THRESHOLD) {
      Serial.println(" -> Status: SUBMERGED");
      client.publish("sensors/moisture", "WET");
    } else {
      Serial.println(" -> Status: DRY");
      client.publish("sensors/moisture", "DRY");
    }

    Serial.println("-------------------------------------------------");
  }
}