// slave 
#include <ESP8266WiFi.h>

#include <ArduinoMqttClient.h>
#include <Grandeur.h>

#include<DHT.h>


const char* wifiSsid = "kebab404";
const char* wifiPassword = "pingpong818";

// mqtt broker
const char* mqttUrl = "mqtt.api.grandeur.tech";
int port = 1883;

const char* mqttUsername = "grandeurlsiw9lz61aex0oigf0dc5ee4"; // API key
const char* mqttPassword = "f274d70a18d1b69caab60e0212c829fd85801004036f3aeefccf81bec8091015"; // Device token

String deviceID = "devicelsiw9ys61af00oigbr9p7f9a";

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define LEDPIN 14

float prev = 0.0;
float temp = 0.0;
int toggleLed = 0;
String toggleMessage;


WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  dht.begin();
  pinMode(LEDPIN, OUTPUT);

  connectWiFi();
  connectMQTT();
  
  // subscribing to ("deviceljel151y45z60jjhhz26detv/temp") topic
  mqttClient.subscribe(deviceID + "/temp");
  mqttClient.subscribe(deviceID + "/toggleLed");

  // recieve message callback function
  mqttClient.onMessage(onMqttMessage);
}

void loop() {
  // call poll() regularly to allow the library to receive MQTT messages and
  // send MQTT keeps alive which avoids being disconnected by the broker
  mqttClient.poll();

  temp = dht.readTemperature();
  
  // topic set up
  mqttClient.beginMessage(deviceID + "/temp");  // topic -> (deviceID + "/temp")
  // publishes temp data to topic 
  mqttClient.print(temp);
  mqttClient.endMessage();
  

  // master
  // if (temp > 22) {
  //   toggleLed = 1;
  //   mqttClient.beginMessage(deviceID + "/toggleLed");
  //   mqttClient.print(toggleLed);
  //   mqttClient.endMessage();
  // } else {
  //   toggleLed = 0;
  // }
}

void connectWiFi() {
    // Connecting to a WiFi network

  Serial.println(); //printlnbreaks line
  Serial.println();
  Serial.print("Connecting to ");  //print doesnt break line
  Serial.println(wifiSsid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with other WiFi-devices on WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(2000);
}

void connectMQTT() {
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(mqttUrl);

  mqttClient.setUsernamePassword(mqttUsername, mqttPassword);

  if (!mqttClient.connect(mqttUrl, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    delay(100);
    while (1);
  }
  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
}

// void receiveMessage() {
//   // we received a message, print out the topic and contents
//   Serial.print("Received a message with topic '");
//   Serial.print(mqttClient.messageTopic());
//   Serial.println(" bytes:");

//   // use the Stream interface to print the contents
//   while (mqttClient.available()) {
//     Serial.print((char)mqttClient.read());
//   }
//   Serial.println();

//   Serial.println();
// }

void onMqttMessage(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', duplicate = ");
  Serial.print(mqttClient.messageDup() ? "true" : "false");
  Serial.print(", QoS = ");
  Serial.print(mqttClient.messageQoS());
  Serial.print(", retained = ");
  Serial.print(mqttClient.messageRetain() ? "true" : "false");
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  if (mqttClient.messageTopic() == (deviceID + "/temp")) {

    String newTemp = "";

    while (mqttClient.available()) {
      newTemp += (char)mqttClient.read(); // 2 + 2 = 22
    } 

    temp = newTemp.toFloat();
    Serial.println();
    Serial.println(temp);
    if (prev == temp) {
      return;
    }  
  

    prev = temp; // prev = 40

    if (temp > 22.0) {
    
      mqttClient.beginMessage(deviceID + "/toggleLed");
      mqttClient.print(1);
      mqttClient.endMessage();
    } else {
      mqttClient.beginMessage(deviceID + "/toggleLed");
      mqttClient.print(0);
      mqttClient.endMessage();
    }
  }


  if (mqttClient.messageTopic() == (deviceID + "/toggleLed")) {

    toggleMessage = "";
     
    while (mqttClient.available()) {
      toggleMessage = (char)mqttClient.read();
      Serial.print(toggleMessage);
    }
  
  
    if (toggleMessage.equals("1")) {
      digitalWrite(LEDPIN, HIGH);
    } else {
       digitalWrite(LEDPIN, LOW);
    }
  }
}

