

#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPmDNS.h>
#include "DHT.h"

// biblioteki
#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 22
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float asoilmoist=analogRead(32);
// logowanie wifi
const char* ssid = "FunBox-42F1";
const char* password = "123456789";

// ip broker mqtt
const char* mqtt_server = "192.168.1.27";
WiFiClient espClient;
PubSubClient client(espClient);
long now = millis();
long lastMeasure = 0;

void setup() {
    dht.begin();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);
 

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
}
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected"); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP32Client");
 
  now = millis();
  float soilmoist=0.95*asoilmoist+0.05*analogRead(32);

  char humString[8];
  dtostrf(soilmoist, 1, 2, humString);
  if (now - lastMeasure > 30000) {
    lastMeasure = now; 
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    float hic = dht.computeHeatIndex(t, h, false);
    static char temperatureTemp[7];
    dtostrf(hic, 6, 2, temperatureTemp);
    static char humidityTemp[7];
    dtostrf(h, 6, 2, humidityTemp);

    // publikowanie
    client.publish("outside/temperature", temperatureTemp);
    client.publish("outside/humidity", humidityTemp);
    client.publish("outside/soilmoist", humString);
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t Heat index: ");
    Serial.print(hic);
    Serial.println(" *C ");

  }
} 
