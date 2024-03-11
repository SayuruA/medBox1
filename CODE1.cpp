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

}


void loop() {

    update_time_with_check_alarm();
    // check for MENU/ OK button
    if (digitalRead(PB_OK) == LOW){
        delay(200);
        go_to_menu();
    }
    check_temp();

}
