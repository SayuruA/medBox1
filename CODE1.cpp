///Libraries and Definitions
#include <Wire.h> //for I2C comm
// libraries required for the oled programming
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// specify screen dimensions in pixel count
#define SCRN_WIDTH 128
#define SCRN_HEIGHT 64

// rst of te screen is as same as the esp-32 board
#define OLED_RST -1
#define SCRN_ADDR 0x3c
///

//display object
Adafruit_SSD1306 display(SCRN_WIDTH, SCRN_HEIGHT, &Wire, OLED_RST);

// time varibles
int days;
int hours;
int minutes;
int seconds; 
unsigned long  timeNow;
unsigned long  timeLast;




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
    
    printOLED(String(days));
    printOLED(":",false,20,0);
    printOLED(String(hours),false,30,0);
    printOLED(":",false,50,0);
    printOLED(String(minutes),false,60,0);
    printOLED(":",false,80,0);
    printOLED(String(seconds),false,90,0);
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
    //



    printOLED("Welcome to the MediBox");


  
}

void loop() {

    update_time();
    printTime();    

}