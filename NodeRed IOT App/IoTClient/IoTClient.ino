#include <ESP8266WiFi.h>
// #include <WiFiUdp.h>
#include <PubSubClient.h>

#define LED 2           // Led in NodeMCU at pin GPIO16 (D0).

WiFiClient espClient;
PubSubClient mqttClient(espClient);

const char* ssid = "iPhone";
const char* password = "abc12345";
int warnPresentage;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  setupMqtt();
}

void loop() {
  if (!mqttClient.connected()){
    connectToBrocker();
  } 
  mqttClient.loop();
  // mqttClient.publish("ENTC-SC-IOT-995", "Hello from ESP32");
  if (warnPresentage > 0) {
    int val = map(warnPresentage, 0, 100, 5000, 500);
    digitalWrite(LED, HIGH);
    delay(val);  
    digitalWrite(LED, LOW);
    delay(val);
    Serial.println("LED is blinking at " + String(val) + "seconds");  
  } else {
    digitalWrite(LED, LOW);
    delay(1000);    
  }
}






