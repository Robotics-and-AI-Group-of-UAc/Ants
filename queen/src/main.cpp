#include <Arduino.h>
#include "ESP32MotorControl.h"
#include "wifi/WiFiMulti.h"
#include "PubSubClient.h"
#include "Sensorultra.h"

#define LEFT 34
#define CENTER 35
#define RIGHT 32

int dt = 0;
// Initialize motors library
ESP32MotorControl MotorControl = ESP32MotorControl();

// Ultrasonic Instance (echo, trigger)
Sensorultra sensorsonic(22, 23);

const char *SSID = "LabTech";
const char *PWD = "#*1wesDI";

// Robot Name - Must be Unique
const char *ROBOT_NAME = "Ultra1";

// MQTT client
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

#define mqttServer "10.1.32.139"
int mqttPort = 1883;

int state = 0;
int state_dt = 1;

int period = 2000;
unsigned long time_left = 0;
unsigned long time_right = 0;

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
  String messageTemp = "";         //
  for (int i = 0; i < length; i++) //
  {
    // Serial.print((char) message[i]);
    messageTemp += (char)message[i];
  }
  if (String(topic) == "aviso_master")
  {
    if (messageTemp == "go")
    {
      state = 1;
    }
    else
    {
      state = 0;
    }
  }
}

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
  delay(500);
  Serial.println("Starting...");
  mqttClient.loop();
}

void reconnect()
{
  // Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected())
  {
    // Serial.println("Reconnecting to MQTT Broker..");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str()))
    {
      // Serial.println("Connected.");
      //  subscribe to topic
      mqttClient.subscribe("aviso_master");
    }
  }
}

void loop()
{
  if (!mqttClient.connected())
    reconnect();
  mqttClient.loop();
  if (state ==1){
  int dt = sensorsonic.ultradistancia();
  if (dt < 10)
  {
    state = 0;
  }
  else
  {
    state = 2;
  }

  if (state == 0)
  {
    MotorControl.motorStop(0);
    MotorControl.motorStop(1);
    mqttClient.publish("aviso", "stop");    
  }

  if (state == 2)
  {    
      mqttClient.publish("aviso", "yey");
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
}