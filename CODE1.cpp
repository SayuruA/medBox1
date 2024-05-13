// author: sayuruA/ 210041M
// project: Smart MediBox/ EN2853



/// Libraries and Definitions *************************************************************************************************************


// for I2C comm
#include <Wire.h> 
// for the oled programming
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// for the Thermal/ Humidity sensor DHT22 
#include <DHTesp.h>
#include <WiFi.h>
// for servo
#include <ESP32Servo.h>
// for MQTT
#include <PubSubClient.h>
// for wifi and Mqtt
WiFiClient espClient;               // wifi client,  provides wifi signal (ininstance)
PubSubClient mqttClient(espClient); // using the wifi instance,  creates an object named mqttClient of type PubSubClient, which is a library used for MQTT communication.

// LDR
#define LDR_1 36 // right 
#define LDR_2 39 // left 

// servo motor
#define motorPin 17 // servo motor

// for the motorangle calculations
String msg;
float motor_angle_offsets;
float control_factor = 0.75; // controlling factor
float min_angle = 30;        // minimum angle
float motor_angle = 0;
float D_servo;

// needed to calculate light intensity
float GAMMA = 0.75;
const float RL10 = 50;
float MIN_ANGLE = 30;
float LUX1 = 0;
float LUX2 = 0;

String temp_value; // needed to save tempreature values

// character arrays needed for publishing data to mqtt server
char LDR_ar[6];
char temp_ar[6];
char high_ldr[38];

// objects
Servo servo; // setup servor motor


// specify screen dimensions in pixel count
#define SCRN_WIDTH 128
#define SCRN_HEIGHT 64

// rst of te screen is as same as the esp-32 board
#define OLED_RST -1
#define SCRN_ADDR 0x3c

// indicators, output pins
#define BUZZER 5
#define LED_1 15
#define PB_CANCEL 34 //indicator stop button

// navigate buttons, output pins
#define PB_OK 32
#define PB_UP 33
#define PB_DOWN 35

// DHT 22, input pin
#define DHTPIN 12

// NTP server
#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET_DST 0   


///**************************************************************************************************************************************///

/// Global Varibles ***********************************************************************************************************************


// oled display object
Adafruit_SSD1306 display(SCRN_WIDTH, SCRN_HEIGHT, &Wire, OLED_RST);
// DHT22 sensor object
DHTesp dhtSensor;

// time keeping varibles
int days;
int hours;
int minutes;
int seconds; 
unsigned long  timeNow;
unsigned long  timeLast;
int UTC_OFFSET = 5*3600+30*60;// UTC offset for Sri lanka (5 30)
// alarm variables
bool alarm_enabled = true; // alarms enbled at the default times(below) in the begining
int n_alarms = 3;
int alarm_hours[] = {0, 7, 19};//specify default alarms
int alarm_minutes[] = {1, 0, 0};
bool alarm_triggered[] = {false, false, false};

// buzzer variables/ tones 
int n_notes = 8;
int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 494;
int C_H = 523;
int notes[] = {C, D, E, F, G, A, B, C_H};

// mode selection varibles
int current_mode = 0;
int max_modes = 6;
String modes[] = {"1 - Set Time", "2 - Set Alarm 1", "3 - Set Alarm 2", "4 - Set Alarm 3", "5 - Disable Alarms", "6 - Set Time Zone"};


const char *MQTT_SERVER = "test.mosquitto.org";
// Producer topics
const char *LDR_TOPIC = "CURRENT_LDR_210035A";
const char *SERVO_MOTOR_ANGLE_TOPIC = "SERVO_MOTOR_ANGLE";
const char *TEMPERATURE_TOPIC = "CURRENT_TEMP_210035A";
const char *MAX_LDR_TOPIC = "MAX_LDR_210035A";

// Subscribe topics
const char *SERVO_MIN_ANGLE_TOPIC = "MIN_ANGLE_210035A";
const char *SERVO_CONTROL_FACTOR_TOPIC = "CONTROL_FACTOR_210035A";
///**************************************************************************************************************************************///


/// Modules/ Functions ********************************************************************************************************************

void printOLED(String text, bool clear = true, int x=0, int y=0, int textSize = 1){
    // clear the oled
    if( clear ){ display.clearDisplay(); }

    //diplay a message
    display.setTextSize(textSize);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(x,y);
    display.println(text);
    display.display();
}

void update_time_with_check_alarm(){
    update_time();
    printTime();
    if (alarm_enabled == true){
        for (int i = 0; i < n_alarms; i++){
            if (alarm_triggered[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes){
                ring_alarm("Medicine Time!");
                alarm_triggered[i] = true;
            }
        }
    }
}

void update_time(){

    struct tm timeinfo;
    getLocalTime(&timeinfo);
    char  timeHour[3];
    strftime(timeHour,3,"%H", &timeinfo);
    hours = atoi(timeHour);
    char  timeMinute[3];
    strftime(timeMinute,3,"%M", &timeinfo);
    minutes = atoi(timeMinute);
    char  timeSecond[3];
    strftime(timeSecond,3,"%S", &timeinfo);
    seconds = atoi(timeSecond);
    char  timeDay[3];
    strftime(timeDay,3,"%d", &timeinfo);
    days = atoi(timeDay);
}

void printTime(){
    
    
    printOLED(String(days)+":"+String(hours)+":"+String(minutes)+":"+String(seconds) , true, 8, 50, 2);
    
    // printOLED(":",false,20,0);
    // printOLED(String(hours),false,30,0);
    // printOLED(":",false,50,0);
    // printOLED(String(minutes),false,60,0);
    // printOLED(":",false,80,0);
    // printOLED(String(seconds),false,90,0);
}

void ring_alarm(String text){
    
    printOLED(text,true,0,0,2);
    // turning the LED ON
    digitalWrite(LED_1, HIGH);
    
    // ringing the buzzer
    bool break_happened = false;
    while ( !(break_happened) && digitalRead(PB_CANCEL) == HIGH ){
        for (int i = 0; i < n_notes; i++){
            if (digitalRead(PB_CANCEL) == LOW){
                delay(200); // to prevent bouncing effect of the push button.
                break_happened = true;
                break;
            }
            ring_buzzer(i);
        }
    }
    
    // turn off the alarm
    digitalWrite(LED_1, LOW);
    display.clearDisplay();
}

void ring_buzzer(int i){

    tone(BUZZER, notes[i]);
    delay(500);
    noTone(BUZZER);
    delay(2);

}

void go_to_menu(){
    while (digitalRead(PB_CANCEL) == HIGH){
        printOLED(modes[current_mode]);
        int pressed = wait_for_button_press();
        
        if (pressed == PB_UP){
            delay(200);
            current_mode++;
            current_mode = current_mode % max_modes;
        }
        else if (pressed == PB_DOWN){
            delay(200);
            current_mode--;
            current_mode = current_mode % max_modes;
            if (current_mode < 0){
                current_mode = max_modes - 1;
            }
        }
        else if (pressed == PB_OK){
            delay(200);
            run_mode(current_mode);
            // Serial.println(current_mode); //Note that the
        }
        else if (pressed == PB_CANCEL){
            delay(200);
            break;
        }
    }
}

int wait_for_button_press(){
    while (true){
        if (digitalRead(PB_UP) == LOW){
            delay(200);
            return PB_UP;
        }
        else if (digitalRead(PB_DOWN) == LOW){
            delay(200);
            return PB_DOWN;
        }
        else if (digitalRead(PB_OK) == LOW){
            delay(200);
            return PB_OK;
        }
        else if (digitalRead(PB_CANCEL) == LOW){
            delay(200);
            return PB_CANCEL;
        }
        //update_time();
    }
}

void run_mode(int mode){
    if (mode == 0){
        set_time();
    }
    else if (mode == 1 || mode == 2 || mode ==3){
        set_alarm(mode - 1); 
    }
    else if (mode == 4){
        alarm_enabled = false;
        printOLED("Alarms disabled!");
        delay(1000);
    }
    else if (mode  == 5){
        config_TZ();
    }
}

void set_time(){
    int temp_hour = hours;
    while (true){
        
        printOLED("Enter hour: " + String(temp_hour));
        int pressed = wait_for_button_press();
        if (pressed == PB_UP){
            delay(200);
            temp_hour++;
            temp_hour = temp_hour % 24;
        }
        else if (pressed == PB_DOWN){
            delay(200);
            temp_hour--;
            temp_hour = temp_hour % 24;
            if (temp_hour < 0){
                temp_hour = 23;
            }
        }
        else if (pressed == PB_OK){
            delay(200);
            hours = temp_hour;
            break;
        }
        else if (pressed == PB_CANCEL){
            delay(200);
            break;
        }
    }
    
    int temp_minute = minutes;
    while (true){
        
        printOLED("Enter minute: " + String(temp_minute));
        int pressed = wait_for_button_press();
        if (pressed == PB_UP){
            delay(200);
            temp_minute++;
            temp_minute = temp_minute % 60;
        }
        else if (pressed == PB_DOWN){
            delay(200);
            temp_minute--;
            temp_minute = temp_minute % 60;
            if (temp_minute < 0){
                temp_minute = 59;
            }
        }
        else if (pressed == PB_OK){
            delay(200);
            minutes = temp_minute;
            break;
        }
        else if (pressed == PB_CANCEL){
            delay(200);
            break;
        }
    }
   
    printOLED("Time is set");
    delay(1000);
}

void set_alarm(int alarm){
    int temp_hour = alarm_hours[alarm];
    while (true){
        // get the hour of the alarm time
        printOLED("Enter hour: " + String(temp_hour));
        int pressed = wait_for_button_press();
        if (pressed == PB_UP){
            delay(200);
            temp_hour++;
            temp_hour = temp_hour % 24;
        }
        else if (pressed == PB_DOWN){
            delay(200);
            temp_hour--;
            temp_hour = temp_hour % 24;
            if (temp_hour < 0){
                temp_hour = 23;
            }
        }
        else if (pressed == PB_OK){
            delay(200);
            alarm_hours[alarm] = temp_hour;
            break;
        }
        else if (pressed == PB_CANCEL){
            delay(200);
            return;
        }
    }
    
    int temp_minute = alarm_minutes[alarm];
    
    while (true){ 
        // get the minute of the alarm time 
        printOLED("Enter minute: " + String(temp_minute));
        int pressed = wait_for_button_press();
        if (pressed == PB_UP){
            delay(200);
            temp_minute++;
            temp_minute = temp_minute % 60;
        }
        else if (pressed == PB_DOWN){
            delay(200);
            temp_minute--;
            temp_minute = temp_minute % 60;
            if (temp_minute < 0){
                temp_minute = 59;
            }
        }
        else if (pressed == PB_OK){
            delay(200);
            alarm_minutes[alarm] = temp_minute;
            
            alarm_enabled = true; // in case there had been a alarm disable, new alarms set after a disable will trigger 
            alarm_triggered[alarm] = false; // setting an alarm again after it has gone off
            
            break;
        }
        else if (pressed == PB_CANCEL){
            delay(200);
            return;
        }
    }
    // print alarm is set to the given time 
    printOLED("Alarm " + String(alarm + 1) + " Set to " + String(temp_hour) + ":" + String(temp_minute));
    delay(4000);
}

void config_TZ(){

    // UTC offeset is given in seconds
    // get the default UTC offset's hours and minutes

    int temp_offset_hours = UTC_OFFSET/3600;
    int temp_offset_minutes = UTC_OFFSET/60 - temp_offset_hours*60;
    
    while (true){
        
        printOLED("Enter UTC offset hours: " + String(temp_offset_hours));
        int pressed = wait_for_button_press();
        if (pressed == PB_UP){
           delay(200);
           temp_offset_hours++;
           if (temp_offset_hours>14){ temp_offset_hours = -12;}
        }

        else if (pressed == PB_DOWN){
            delay(200);
            temp_offset_hours--;
            if (temp_offset_hours < -12){
               temp_offset_hours = 14;
            }
        }
        else if (pressed == PB_OK){
            delay(200);   
            break;          
        }
        else if (pressed == PB_CANCEL){
            delay(200);   
            return;// no need to run further once cancel is pressed
        }
    }
    while (true)
    {
        printOLED("Enter UTC offset minutes: " + String(temp_offset_minutes));
        int pressed = wait_for_button_press();
        if (pressed == PB_UP){
            delay(200);
            temp_offset_minutes++;
            temp_offset_minutes = temp_offset_minutes % 60;
        }
        else if (pressed == PB_DOWN){
            delay(200);
            temp_offset_minutes--;
            temp_offset_minutes = temp_offset_minutes % 60;
            if (temp_offset_minutes < 0)
            {
                temp_offset_minutes = 59;
            }
        }
        else if (pressed == PB_OK){
            delay(200);
            UTC_OFFSET = temp_offset_hours*3600 + temp_offset_minutes*60;
            break;
        }
        else if (pressed == PB_CANCEL){
            delay(200);
            return;
        }
    }
    configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
    printOLED("Time zone is set to " + String(temp_offset_hours) + " " +String(temp_offset_minutes));
    delay(4000);
}

void check_temp(){
    TempAndHumidity data = dhtSensor.getTempAndHumidity();

    if (data.temperature > 32){    
        printOLED("HIGH TEMP (" + String(data.temperature) + ")");        
        ring_buzzer(7);
        delay(300);
        ring_buzzer(7);
        delay(1000);
        
    }
    
    else if (data.temperature < 26){
        printOLED("LOW TEMP (" + String(data.temperature) + ")");
        ring_buzzer(6);
        delay(300);
        ring_buzzer(6);
        delay(1000);
    }
    
    // for humidity
    if (data.humidity > 80){
        printOLED("HUMIDITY HIGH ("+ String(data.humidity) + "%" + ")");
        ring_buzzer(5);
        delay(300);
        ring_buzzer(5);
        delay(1000);
    }
    else if (data.humidity < 60){
        printOLED("HUMIDITY LOW ("+ String(data.humidity) + "%" + ")");
        ring_buzzer(4);
        delay(300);
        ring_buzzer(4);
        delay(1000);delay(1000);
    }
}

// function for establishing mqtt connection
void setupMqtt()
{
  mqttClient.setServer("test.mosquitto.org", 1883); // setup mqtt server, "test.mosquitto.org"
  mqttClient.setCallback(receiveCallback);          // after subscribing, reciver call back recieves values
}

// function to attempt connection to MQTT broker and subscribes to topics
void conect_to_broker()
{

  // check whether connected, and while it is not connected.....
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection..."); //....print connecting
    if (mqttClient.connect("ESP-8266-67895684567"))
    {                            // ESP-8266-67895684567 is the id
      Serial.print("connected"); // srver connection success

      // subscribe to the below topics of Node red
      mqttClient.subscribe(SERVO_CONTROL_FACTOR_TOPIC);
      mqttClient.subscribe(SERVO_MIN_ANGLE_TOPIC); // best place to subscribe (incoming) is in connect to broker
    }

    //....if mqtt connection failed
    else
    {
      Serial.print("failed; client state:");
      Serial.println(mqttClient.state()); // mqtt client state gives what the problem in connection is
      delay(3000);                        // wait 5s before attempting reconnection
    }
  }
}

// function for creating and selecting the maximum light inensity values
void light_intensity(){
  calculate_LDR_values(); // calculte LUX1 & LUX2
  String str_temp;

  // checking if intensity values are nan and if so, setting them to zero
  // if nan, we can not compare
  if (isnan(LUX1)){
    LUX1 = 0;
  }
  if (isnan(LUX2)){
    LUX2 = 0;
  }

  // comparing maximum light intensity values
  // right ldr is higher
  if (LUX1 > LUX2){
    String(LUX1).toCharArray(LDR_ar, 6);
    str_temp = "Left LDR < Right LDR";
    str_temp.toCharArray(high_ldr, 38);
  }
  // left ldr is higher
  else if (LUX2 > LUX1){
    String(LUX2).toCharArray(LDR_ar, 6);
    str_temp = "Left LDR > Right LDR";
    str_temp.toCharArray(high_ldr, 38);
  }
  // both ldrs is have same value
  else{
    str_temp = "LDRs : Same Intensity";
    str_temp.toCharArray(high_ldr, 38);
  }
}

// funtion to calculate motor angle
void sliding_window(){
  calculate_LDR_values(); // calculte LUX1 & LUX2
  if (isnan(LUX1)){
    LUX1 = 0;
  }
  if (isnan(LUX2)){
    LUX2 = 0;
  }

  if (LUX2 < LUX1){
    D_servo = 1.5; // given in assignment
  }
  else if (LUX2 > LUX1){
    D_servo = 0.5; // given in assignment
  }

  float max_intensity = max(LUX1, LUX2); // take the maximum of the 2 intensities as the maximum light intensity
  if (min_angle != 0 && control_factor != 0){
    motor_angle_offsets = min_angle;
    motor_angle = min((motor_angle_offsets * D_servo) + (180 - motor_angle_offsets) * max_intensity * control_factor, (float)180); // calculate motor angel using the equation
    Serial.print("motor_angle: ");
    Serial.println(motor_angle);
    servo.write(motor_angle); // set the servo motor's angel to the calculated value
    delay(15);
  }
}

// function to calculate maximum light intensities in 0-1 range
void calculate_LDR_values(){

  float volt_1 = analogRead(LDR_1) / 1024. * 5;
  float R1 = 2000 * volt_1 / (1 - volt_1 / 5);
  float max_LUX1 = pow(RL10 * 1e3 * pow(10, GAMMA) / 322.58, (1 / GAMMA)); // max intensity of light of right ldr
  LUX1 = pow(RL10 * 1e3 * pow(10, GAMMA) / R1, (1 / GAMMA)) / max_LUX1;    // max light intensity (R) range 0-1

  float volt_2 = analogRead(LDR_2) / 1024. * 5;
  float R2 = 2000 * volt_2 / (1 - volt_2 / 5);
  float max_LUX2 = pow(RL10 * 1e3 * pow(10, GAMMA) / 322.58, (1 / GAMMA)); // calculate max intensity of light of left ldr
  LUX2 = pow(RL10 * 1e3 * pow(10, GAMMA) / R2, (1 / GAMMA)) / max_LUX2;    // max light intensity (L) range 0-1
}

// function to convert temreature values to character array
void temp_change_to_char(void){
  TempAndHumidity data = dhtSensor.getTempAndHumidity(); // get temp and humid values
  delay(100);
  temp_value = String(data.temperature, 2); // convert tempreature value of 2 decimal places to string
  temp_value.toCharArray(temp_ar, 6);       // convert string into char array
}

// Receiving data from the server
void receiveCallback(char *topic, byte *payload, unsigned int length){
  Serial.print("message arrived [");
  Serial.print(topic); // the topic of the rxed msg will display
  Serial.print("] ");

  char payloadCharAr[length]; // the payload that was recived, saved in payloadcharar, with the coming data size
  for (int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
    payloadCharAr[i] = (char)payload[i]; // convert each incoming data payload into character
  }
  Serial.println(); // print the recived data

  String str_msg; // create a string variable

  // if the recived message topic is SERVO_MIN_ANGLE_TOPIC
  if (strcmp(topic, SERVO_MIN_ANGLE_TOPIC) == 0){
    for (int i = 0; i < length; i++){
      str_msg += (char)payload[i];   // create the string str_msg by concatinating each charcter of payload
      min_angle = str_msg.toFloat(); // generate minimum angle floating point value, and set it to min_angle variable
      delay(100);                    // Add delay for stabilization
    }
    Serial.print("Min angle: ");
    Serial.println(min_angle);
  }

  // if incoming topic is SERVO_CONTROL_FACTOR_TOPIC
  if (strcmp(topic, SERVO_CONTROL_FACTOR_TOPIC) == 0){
    for (int i = 0; i < length; i++){
      str_msg += (char)payload[i];        // create the string str_msg by concatinating each charcter of payload
      control_factor = str_msg.toFloat(); // generate control factor floating point value, and set it to control_factor variable
      delay(100);                         // Add delay for stabilization
    }
    Serial.print("Control factor: ");
    Serial.println(control_factor);
  }
}


//////////////////////////***************************************************************************************************************///

void setup() {


    //setup indicator pins
    pinMode(BUZZER, OUTPUT);
    pinMode(LED_1, OUTPUT);

    // cancel /alarm stop button
    pinMode(PB_CANCEL, INPUT);
    
    // navigation buttons
    
    pinMode(PB_OK, INPUT);
    pinMode(PB_UP, INPUT);
    pinMode(PB_DOWN, INPUT);
    
    // initialize DHT 22
    dhtSensor.setup(DHTPIN, DHTesp::DHT22);
    

    //initialize serial monitor and the oled
    Serial.begin(115200);
    if (!display.begin( SSD1306_SWITCHCAPVCC, SCRN_ADDR ) ){
        Serial.println( F( "SSD1306 allocation failed" ) );
        for(;;);
    }

    // turn on the oled
    display.display();
    delay(2000);
    
    // connect to WiFi
    WiFi.begin("Wokwi-GUEST", "", 6);
    while (WiFi.status() != WL_CONNECTED){
        delay(250);
        printOLED("Connecting to WIFI");
    }
    // get time from NTP
    configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);

    printOLED("Connected to WIFI");
    delay(1000);

    printOLED("Welcome to MediBox!");
    delay(1000);

    servo.attach(motorPin, 500, 2400);
    setupMqtt();

}


void loop() {

    update_time_with_check_alarm();
    // check for MENU/ OK button
    if (digitalRead(PB_OK) == LOW){
        delay(200);
        go_to_menu();
    }
    check_temp();

    if (!mqttClient.connected()){
        Serial.println("inside calling connect to broker");
        conect_to_broker(); // if not connected, connect to mqtt using connect_to_broker() function
    }

    mqttClient.loop(); // ensure that the MQTT client continuously processes any incoming messages, publishes any outgoing messages, and maintains communication with the MQTT broker.

    // publishes any available values while looping
    mqttClient.publish(LDR_TOPIC, LDR_ar);
    mqttClient.publish(MAX_LDR_TOPIC, high_ldr);
    mqttClient.publish(TEMPERATURE_TOPIC, temp_ar);

    delay(1000);

    temp_change_to_char();
    light_intensity();
    sliding_window();
}
