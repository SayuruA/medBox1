///Libraries and Definitions

//for I2C comm
#include <Wire.h> 
// libraries required for the oled programming
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// specify screen dimensions in pixel count
#define SCRN_WIDTH 128
#define SCRN_HEIGHT 64

// rst of te screen is as same as the esp-32 board
#define OLED_RST -1
#define SCRN_ADDR 0x3c

// indicators
#define BUZZER 5
#define LED_1 15
#define PB_CANCEL 34 //indicator stop button
///

//oled display object
Adafruit_SSD1306 display(SCRN_WIDTH, SCRN_HEIGHT, &Wire, OLED_RST);

// time varibles
int days;
int hours;
int minutes;
int seconds; 
unsigned long  timeNow;
unsigned long  timeLast;

// alarm variables
bool alarm_enabled = true;
int n_alarms = 3;
int alarm_hours[] = {0, 1,2};
int alarm_minutes[] = {1, 10,20};
bool alarm_triggered[] = {false, false, false};

// buzzer tones
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

void printTime(){
    
    
    printOLED(String(days)+":"+String(hours)+":"+String(minutes)+":"+String(seconds));
    
    // printOLED(":",false,20,0);
    // printOLED(String(hours),false,30,0);
    // printOLED(":",false,50,0);
    // printOLED(String(minutes),false,60,0);
    // printOLED(":",false,80,0);
    // printOLED(String(seconds),false,90,0);
}

void update_time(){
    timeNow = millis()/1000;
    seconds = timeNow-timeLast;

    if (seconds >= 60){
        minutes += 1;
        timeLast+=60;
        seconds = 0;
    }
    if (minutes == 60){
        hours+=1;
        minutes=0;
    }
    if (hours == 24){
        days += 1;
        hours = 0;
    }
}
void update_time_with_check_alarm(){
    update_time();
    printTime();
    if (alarm_enabled == true){
        for (int i = 0; i < n_alarms; i++){
            if (alarm_triggered[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes){
                ring_alarm();
                alarm_triggered[i] = true;
            }
        }
    }
}

void ring_alarm(){
    printOLED("MEDICNE TIME!");
    // turning the LED ON
    digitalWrite(LED_1, HIGH);
    // ringing the buzzer
    bool break_happened = false;
    while ( !(break_happened) && digitalRead(PB_CANCEL) == HIGH ){
        for (int i = 0; i < n_notes; i++){
            if (digitalRead(PB_CANCEL) == LOW){
                delay(200); // to prevent bouncing of the push button.
                break_happened = true;
                break;
            }
            tone(BUZZER, notes[i]);
            delay(500);
            noTone(BUZZER);
            delay(2);
        }
    }
    digitalWrite(LED_1, LOW);
    display.clearDisplay();
}


void setup() {
    
    //initialize serial monitor and the oled
    Serial.begin(115200);
    if (!display.begin( SSD1306_SWITCHCAPVCC, SCRN_ADDR ) ){
        Serial.println( F( "SSD1306 allocation failed" ) );
        for(;;);
    }
    // turn on the oled
    display.display();
    delay(2000);
    
    //setup indicator pins
    pinMode(BUZZER, OUTPUT);
    pinMode(LED_1, OUTPUT);
    // cancel /alarm stop button
    pinMode(PB_CANCEL, INPUT);

    printOLED("Welcome to the MediBox");


  
}

void loop() {

    update_time_with_check_alarm();

}
