void setupMqtt() {
  mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setCallback(recieveCallBack);
}

void recieveCallBack(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived from : ");
  Serial.println(topic);
  Serial.print("Message : ");

  char payloadCharAr[length];

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);  
    payloadCharAr[i] = (char)payload[i];
  }
  Serial.println();
    
  payloadCharAr[length] = '\n'; 
  warnPresentage = atoi(payloadCharAr);
}


void connectToBrocker() {
  while(!mqttClient.connected()) {
    Serial.println("Attempting to connect");
    if(mqttClient.connect("ESP32-4324234234")) {
      Serial.println("Connected to Broker");
      mqttClient.subscribe("ENTC-SC-IOT-WARN-PRESENTAGE");      
    } else {
      Serial.println("Error...");       
      Serial.println(mqttClient.state());
      delay(5000);       
    }
  }
}