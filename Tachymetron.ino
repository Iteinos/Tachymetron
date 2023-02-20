//Display Dependencies
#include <MD_Parola.h>  //matrix GFX library
#include <MD_MAX72xx.h> //matrix control library
#include <SPI.h>
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 2   // 7219 chain number
#define CLK_PIN 14      // SCK
#define DATA_PIN 13     // MOSI
#define CS_PIN 12       // CS
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
//GPS Dependencies
#include <NMEAGPS.h>
#include <SoftwareSerial.h>
SoftwareSerial gpsPort(5, 4);
#include <Streamers.h>
static NMEAGPS gps; //GPS module instance
static gps_fix fix; //Coordinate instance

void setup() {
  ESP.wdtDisable();   //disable esp8266 software watchdog timer
  *((volatile uint32_t *)0x60000900) &= ~(1);   //disable esp8266 hardware watchdog timer
  Serial.begin(115200); //debug serial init
  P.begin();  //gfx instance
  mx.begin(); //control instance
  mx.control(MD_MAX72XX::TEST, MD_MAX72XX::ON);
  mx.control(MD_MAX72XX::INTENSITY, 15);  //display luminosity control 0-15
  delay(200);
  mx.control(MD_MAX72XX::TEST, MD_MAX72XX::OFF);
  gpsPort.begin(9600); //soft UART init
}

bool blink = 0;
void displicate(int content) {    //display
  if(!fix.status){      //when satellites are not located
    if(blink) P.displayText("--", PA_CENTER, 100, 100, PA_PRINT);
    else P.displayText("", PA_CENTER, 100, 100, PA_PRINT);  //blinking cursor effect
    blink = 1 - blink;
    P.displayAnimate(); //refresh 
  }else{
    char c[8];  //content buffer
    String(content).toCharArray(c, 7);  //write to buffer
    P.displayText((char *)c, PA_CENTER, 100, 100, PA_PRINT);
    P.displayAnimate();
  }
}

void loop() {
  while (gps.available(gpsPort)) {    //when soft UART is open
    fix = gps.read();   //resolve data
    trace_all(Serial, gps, fix);  //debug serial output
    displicate((int)fix.speed_kph()); //display current speed
  }
}  