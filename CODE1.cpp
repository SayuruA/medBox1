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

Adafruit_SSD1306 display(SCRN_WIDTH, SCRN_HEIGHT, &Wire, OLED_RST);



void setup() {
  
  Serial.begin(115200);
  if (!display.begin( SSD1306_SWITCHCAPVCC, SCRN_ADDR ) ){
    Serial.println( F( "SSD1306 allocation failed" ) );
    for(;;);
  }
  

  // turn on the oled
  display.display();
  delay(2000);

  // clear the oled
  display.clearDisplay();

  //diplay a message
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println( F( "Welcome to Medibox!" ) );
  display.display();
  
}

void loop() {
 
  delay(10); // this speeds up the simulation
}