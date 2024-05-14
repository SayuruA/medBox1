#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>

// LDR modules
#define LDR_LEFT 32
#define LDR_RIGHT 35
// DHT 22, input pin
#define DHTPIN 33

// Gobal variables //

const char *MQTT_SERVER = "test.mosquitto.org";
// Producer topics
const char *LDR_TOPIC = "CURRENT_LDR_210041M";
const char *TEMPERATURE_TOPIC = "TEMP_210041M";
const char *MAX_LDR_TOPIC = "LDR_210041M";

// Subscribe topics
const char *SERVO_MIN_ANGLE_TOPIC = "MIN_ANGLE_041M";
const char *SERVO_CONTROL_FACTOR_TOPIC = "CONTROL_FACTOR_041M";

char tempArr[6];
char ldr[6];
// char ldrLArr[4];
// char ldrRArr[4];

DHTesp dhtsensor;
Servo servo; 

int t_off = 30;
float gamma_i = 0.75;

// for wifi and Mqtt
WiFiClient espClient;               
PubSubClient mqttClient(espClient); 


void setup() {
  
  Serial.begin(115200);
  pinMode(LDR_LEFT,INPUT);
  pinMode(LDR_RIGHT,INPUT);

  dhtsensor.setup(DHTPIN, DHTesp::DHT22);
  WiFi.begin("Wokwi-GUEST", "", 6);

  servo.attach(23);
  setupMqtt();
}

void loop() {

  if (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker");
    connectTOBroker();
  }
  mqttClient.loop();
  updateLight();
  check_temp();
  // Serial.print(ldr);Serial.print(" ");
  // Serial.println(tempArr);
  mqttClient.publish(TEMPERATURE_TOPIC, tempArr);
  //
  // float a = 17.6;
  // String(a,2).toCharArray(ldr, 6);
  //
  mqttClient.publish(MAX_LDR_TOPIC, ldr);
  delay(50);
  
  
  // int a = analogRead(LDR_LEFT);
  // int b = analogRead(LDR_RIGHT);
  // Serial.print(a);
  // Serial.print(" ");
  // Serial.println(b);
  // delay(100);
  
}

void setupMqtt() {
  mqttClient.setServer(MQTT_SERVER, 1883);
  mqttClient.setCallback(recieveCallback);
}

void connectTOBroker() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32Client-210041M")) {
      Serial.println("MQTT Connected");
      //mqttClient.subscribe("ENTC-ON-OFF_NI");
      
      mqttClient.subscribe("SERVO_MIN_ANGLE_TOPIC");
      mqttClient.subscribe("SERVO_CONTROL_FACTOR_TOPIC");
    } else {
      Serial.print("Failed To connect to MQTT Broker");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}

void recieveCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char payloadCharAr[length];
  Serial.print("Message Recieved: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadCharAr[i] = (char)payload[i];
  }
  Serial.println();
  // if (strcmp(topic, "ENTC-ON-OFF_NI") == 0) {
  //   if (payloadCharAr[0] == '1') {
  //     digitalWrite(15, HIGH);
  //   } else {
  //     digitalWrite(15, LOW);
  //   }
  // } else if (strcmp(topic, "ENTC-ADMIN-SCH-ON_NI") == 0) {
  //   if (payloadCharAr[0] == 'N') {
  //     isScheduledON = false;
  //   } else {
  //     isScheduledON = true;
  //     scheduledOnTime = atol(payloadCharAr);
  //   }
    if (strcmp(topic, SERVO_MIN_ANGLE_TOPIC) == 0) {
      t_off = String(payloadCharAr).toInt();

  } else if (strcmp(topic, SERVO_CONTROL_FACTOR_TOPIC) == 0) {
      gamma_i = String(payloadCharAr).toFloat();
  }
}

void updateLight() {

  float lsv = analogRead(LDR_LEFT) * 1.00;
  float rsv = analogRead(LDR_RIGHT) * 1.00;
  // Serial.println(lsv);
  // Serial.println(rsv);

  float lsv_cha = (float)(lsv - 4063.00) / (32.00 - 4063.00);
  float rsv_cha = (float)(rsv - 4063.00) / (32.00 - 4063.00);
  //  Serial.println(String(lsv_cha)+" "+String(rsv_cha));
  
  float max_I = lsv_cha;
  float D = 1.5;

  if (rsv_cha > max_I) {
    max_I = rsv_cha;
    D = 0.5;
  }
  updateAngle(max_I, D);
  
  // String(lsv_cha).toCharArray(ldrLArr, 4);
  // String(rsv_cha).toCharArray(ldrRArr, 4);
}

void updateAngle(float max_I, float D) {
  Serial.println(max_I);
  String(max_I,2).toCharArray(ldr, 6);
  
  int theta = t_off * D + (180 - t_off) * max_I * gamma_i;
  theta = min(theta, 180);

  servo.write(theta);
}

void check_temp() {
  TempAndHumidity data = dhtsensor.getTempAndHumidity();
  ///////////////////////
  String(data.temperature, 2).toCharArray(tempArr, 6);

  /////////////////////////
  
  // if (data.temperature > 32) {
  //   print_line_time("TEMP HIGH", 0, 40, 1);
  // } else if (data.temperature < 26) {
  //   print_line_time("TEMP LOW", 0, 40, 1);
  // }
  // if (data.humidity > 80) {
  //   print_line_time("HUMIDITY HIGH", 0, 50, 1);
  // } else if (data.humidity < 60) {
  //   print_line_time("HUMIDITY LOW", 0, 50, 1);
  // }
  // display.display();

}
