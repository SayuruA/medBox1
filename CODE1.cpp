// author: sayuruA/ 210041M
// project: Smart MediBox/ EN2853



/// Libraries and Definitions *************************************************************************************************************


// for I2C comm
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

// navigate buttons
#define PB_OK 32
#define PB_UP 33
#define PB_DOWN 35

///**************************************************************************************************************************************///

/// Gllobal Varibles ***********************************************************************************************************************


//oled display object
Adafruit_SSD1306 display(SCRN_WIDTH, SCRN_HEIGHT, &Wire, OLED_RST);

// time keeping varibles
int days;
int hours;
int minutes;
int seconds; 
unsigned long  timeNow;
unsigned long  timeLast;

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
    // else if (mode  == 5){
    //     set_time_zone();
    // }
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
        update_time();
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
            break;
        }
    }
    
    int temp_minute = alarm_minutes[alarm];
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
            alarm_minutes[alarm] = temp_minute;
            break;
        }
        else if (pressed == PB_CANCEL){
            delay(200);
            break;
        }
    }
    printOLED("Alarm " + String(alarm));
    delay(1000);
}

//////////////////////////***************************************************************************************************************///

void setup() {
    

    
    //setup indicator pins
    pinMode(BUZZER, OUTPUT);
    pinMode(LED_1, OUTPUT);

    // cancel /alarm stop button
    pinMode(PB_CANCEL, INPUT);
    // navigation buttons
    pinMode(PB_DOWN, INPUT);
    pinMode(PB_OK, INPUT);
    pinMode(PB_UP, INPUT);

    //initialize serial monitor and the oled
    Serial.begin(115200);
    if (!display.begin( SSD1306_SWITCHCAPVCC, SCRN_ADDR ) ){
        Serial.println( F( "SSD1306 allocation failed" ) );
        for(;;);
    }

    // turn on the oled
    display.display();
    delay(2000);

    printOLED("Welcome to MediBox!");


  
}

void loop() {

    
    update_time_with_check_alarm();
    // check for MENU/ OK button
    if (digitalRead(PB_OK) == LOW){
        delay(200);
        go_to_menu();
    }
    
}
