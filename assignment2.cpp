// Author: Sayuru Amugoda/ 210041M
// MediBox Part 2

// Libraries**********************//
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>

//Gloabal Variables****************//
// LDR modules
#define LDR_LEFT 32
#define LDR_RIGHT 35
// DHT 22, input pin
#define DHTPIN 33

// MQTT Server
const char *MQTT_SERVER = "test.mosquitto.org";

// Publishing topics
const char *TEMPERATURE_TOPIC = "TEMP_210041M";
const char *MAX_LDR_TOPIC = "LDR_210041M";
const char *Current_High_LDR = "Current_High_LDR";
// Subscribing topics
const char *SERVO_MIN_ANGLE_TOPIC = "MIN_ANGLE_041M";
const char *SERVO_CONTROL_FACTOR_TOPIC = "CONTROL_FACTOR_041M";

// Char arrays to be published
char tempArr[6];
char ldr[6];
char maxLdrArr[4];

DHTesp dhtsensor;
Servo servo; 

int t_off = 30;
float gamma_i = 0.75;

// for wifi and Mqtt
WiFiClient espClient;               
PubSubClient mqttClient(espClient); 

// Main Program *************************//

void setup() {
  
  Serial.begin(115200);

  // Setting up LDR pins and Servo and DHT22  
  //NOTE: Some analog input pins become disabled by turning on WiFi
  pinMode(LDR_LEFT,INPUT);
  pinMode(LDR_RIGHT,INPUT);
  dhtsensor.setup(DHTPIN, DHTesp::DHT22);
  servo.attach(23);
  
  WiFi.begin("Wokwi-GUEST", "", 6);
  setupMqtt();
}

void loop() {

  // Checking if the MQTT connection is intact 
  if (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker");
    connect2broker();
  }
  // Checking for incoming messages
  mqttClient.loop();
  // Updating the sliding window status and checking temperature
  moveWindow();
  check_temp();
  // Publish the light intensity and temperature data
  mqttClient.publish(TEMPERATURE_TOPIC, tempArr);
  mqttClient.publish(MAX_LDR_TOPIC, ldr);
  mqttClient.publish(Current_High_LDR, maxLdrArr);
  delay(50);

  // CODE USED FOR DEBUGGING // ****END OF MAIN LOOP****
  
  // Serial.print(ldr);Serial.print(" ");
  // Serial.println(tempArr);
  
  //
  // float a = 17.6;
  // String(a,2).toCharArray(ldr, 6);

  // int a = analogRead(LDR_LEFT);
  // int b = analogRead(LDR_RIGHT);
  // Serial.print(a);
  // Serial.print(" ");
  // Serial.println(b);
  // delay(100);
  // ******// 
}

void setupMqtt() {
  mqttClient.setServer(MQTT_SERVER, 1883);
  // Giving the call back instructions to mqttClient class  
  mqttClient.setCallback(callBack);
}

void connect2broker() {
  // Repeating until connected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (mqttClient.connect("ESP32Client-210041M")) {
      Serial.println("MQTT Connected");
      
      // Subscribe to the required topics
      mqttClient.subscribe(SERVO_MIN_ANGLE_TOPIC);
      mqttClient.subscribe(SERVO_CONTROL_FACTOR_TOPIC);
    } else {
      Serial.print("Failed To connect to MQTT Broker");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}

void callBack(char *topic, byte *payload, unsigned int length) {
  // Handling recieved messages
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  //Char array to store the payload
  char payloadCharAr[length];
  Serial.print("Message Recieved: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadCharAr[i] = (char)payload[i];
  }
  Serial.println();

  // Identifying the topic and extracting data accordingly
    if (strcmp(topic, SERVO_MIN_ANGLE_TOPIC) == 0) {
      t_off = String(payloadCharAr).toInt();

  } else if (strcmp(topic, SERVO_CONTROL_FACTOR_TOPIC) == 0) {
      gamma_i = String(payloadCharAr).toFloat();
  }
}

void moveWindow() {
  
  float left = analogRead(LDR_LEFT) * 1.00;
  float right = analogRead(LDR_RIGHT) * 1.00;

  // transforming the data values if "32 - 4063" to "0 - 1"
  float leftChar = (float)(left - 4063.00) / (32.00 - 4063.00);
  float rightChar = (float)(right - 4063.00) / (32.00 - 4063.00);
  
  // Finding the maximum light intensity and accordingly decide D
  float maxIntencity = leftChar;
  float D = 1.5;

  if (rightChar > maxIntencity) {
    maxIntencity = rightChar;
    D = 0.5;
  }
  // Publishing a value to identify the side with the highest light  intensity 
  String(D,2).toCharArray(maxLdrArr, 4);
  
  updateAngle(maxIntencity, D);
}

void updateAngle(float maxIntencity, float D) {
  // Adding the light intensity string to a char array to be published  
  String(maxIntencity,2).toCharArray(ldr, 6);
  // Calculating theta using the formula
  int theta = t_off * D + (180 - t_off) * maxIntencity * gamma_i;
  theta = min(theta, 180);
  // Actuating the servo
  servo.write(theta);
}

void check_temp() {
  
  TempAndHumidity data = dhtsensor.getTempAndHumidity();
  // Adding the temperature string to a char array to be published
  String(data.temperature, 2).toCharArray(tempArr, 6);
}
