#include <Arduino.h>
#include "ESP32MotorControl.h"
#include "wifi/WiFiMulti.h"
#include "PubSubClient.h"

#define LEFT 34
#define CENTER 35
#define RIGHT 32

// Initialize motors library
ESP32MotorControl MotorControl = ESP32MotorControl();

int state = 0;
int period = 2000;
unsigned long time_now = 0;

// Constants connection WIFI (pointer to string)
const char *SSID = "LabTech";
const char *PWD = "#*1wesDI";

// Robot Name - Must be Unique
const char *ROBOT_NAME = "Ultra1";

// MQTT client
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

#define mqttServer "10.1.32.139"
int mqttPort = 1883;

// Connect to WIFI
void connectToWiFi()
{

  Serial.print("Connecting to ");

  WiFi.begin(SSID, PWD);
  Serial.println(SSID);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.print("Connected.");
}

// Receive message from topic (subscribe)
void callback(char *topic, byte *message, unsigned int length)
{
  // Serial.print("Callback - ");
  // Serial.print("Message:");
  String messageTemp = "";         //
  for (int i = 0; i < length; i++) //
  {
    // Serial.print((char) message[i]);
    messageTemp += (char)message[i];
  }
  if (String(topic) == "aviso")
  {
    if (messageTemp == "stop")
    {
      state = 0;
    }
    else{
      state = 1;
    }
    
  }
}

// Setup MQTT Client
void setupMQTT()
{
  mqttClient.setServer(mqttServer, mqttPort);
  // set the callback function
  mqttClient.setCallback(callback);
}

void setup()
{
  MotorControl.attachMotors(17, 18, 26, 27);
  Serial.begin(115200);
  pinMode(LEFT, INPUT);
  pinMode(CENTER, INPUT);
  pinMode(RIGHT, INPUT);


  Serial.println();

  connectToWiFi();
  setupMQTT();
}

// Reconnect if MQTT connection down
void reconnect()
{
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected())
  {
    Serial.println("Reconnecting to MQTT Broker..");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str()))
    {
      Serial.println("Connected.");
      // subscribe to topic
      mqttClient.subscribe("aviso");
    }
  }
}

void loop()
{
  if (!mqttClient.connected())
    reconnect();
  mqttClient.loop();

  if (state == 0)
  {
    MotorControl.motorStop(0);
    MotorControl.motorStop(1);    
  }


  if (state == 1)
  {
    if (digitalRead(LEFT) == 0 && digitalRead(RIGHT) == 0 && digitalRead(CENTER) == 1)
    {
      MotorControl.motorForward(0, 50);
      MotorControl.motorForward(1, 50);
    }
    else if (digitalRead(LEFT) == 1)
    {
      MotorControl.motorForward(1, 55);
      MotorControl.motorReverse(0, 20);
    }
    else if (digitalRead(RIGHT) == 1)
    {
      MotorControl.motorReverse(1, 20);
      MotorControl.motorForward(0, 55);
    }
    else
    {
      MotorControl.motorForward(0, 50);
      MotorControl.motorForward(1, 50);
    }
  }
}