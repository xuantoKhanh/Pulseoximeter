//Libraries for Max30100 sensor
#include "MAX30100_PulseOximeter.h" 
//ESP8266 WiFi main library
#include <ESP8266WiFi.h>  //should be at least 2.4.1
//Libraries for Firebase and network
#include <ESP8266HTTPClient.h>
#include "ESP8266WebServer.h"
#include <Arduino.h> 
#include <FirebaseArduino.h>  //ArduinoJson is at least version 5.13.1
//Libraries for OLED screeen
#include <Adafruit_GFX.h>    //version 1.2.2
#include <Adafruit_SSD1306.h>  //include Adafruit SSD1306 OLED display driver
#include "OakOLED.h"
#include <Wire.h>

// Libraries for internet time
#include <WiFiUdp.h>
#include <NTPClient.h>          // include NTPClient library
#include <TimeLib.h>            // include Arduino time library

#define FIREBASE_HOST "authapp-a22b5-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "5lRCXxRD0DsvYh4aQHaatAP2aGnce3v77tDBVOyZ"
#define WIFI_SSID "TP-LINK_9160"
#define WIFI_PASSWORD "12341234"

WiFiUDP ntpUDP;
// ‘time.nist.gov’ is used (default server) with +1 hour offset (3600 seconds) 60 seconds (60000 milliseconds) update interval
NTPClient timeClient(ntpUDP, "time.nist.gov", 25200, 60000); //25200 là thời gian ở Vn +7


PulseOximeter pox;
OakOLED oled;

uint32_t  tsLastReport = 0;
const long interval    = 30000;
volatile boolean heartBeatDetected = false;

const unsigned char bitmap [] PROGMEM=
{
0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x18, 0x00, 0x0f, 0xe0, 0x7f, 0x00, 0x3f, 0xf9, 0xff, 0xc0,
0x7f, 0xf9, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xe0, 0x7f, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xf7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0x7f, 0xdb, 0xff, 0xe0,
0x7f, 0x9b, 0xff, 0xe0, 0x00, 0x3b, 0xc0, 0x00, 0x3f, 0xf9, 0x9f, 0xc0, 0x3f, 0xfd, 0xbf, 0xc0,
0x1f, 0xfd, 0xbf, 0x80, 0x0f, 0xfd, 0x7f, 0x00, 0x07, 0xfe, 0x7e, 0x00, 0x03, 0xfe, 0xfc, 0x00,
0x01, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x3f, 0xc0, 0x00,
0x00, 0x0f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


void onBeatDetected(){
heartBeatDetected = true;
Serial.println("Beat!");
 oled.drawBitmap(60, 20, bitmap, 28, 28, 1);
 oled.display();
}

void setup(){
Serial.begin(115200);

    oled.begin();
    oled.clearDisplay();
    oled.setTextSize(1.5);
    oled.setTextColor(1);
    oled.setCursor(0, 0);

    pinMode(16, OUTPUT);

    oled.println("Initializing pulse oximeter..");
    oled.display();

WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
Serial.print("Connecting");
while (WiFi.status() != WL_CONNECTED) {
Serial.print(".");
delay(500);
}
Serial.println();
Serial.print("Connected with IP: ");
Serial.println(WiFi.localIP());

Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
if (Firebase.failed()) {
Serial.print("setting /message failed:");
Serial.println(Firebase.error());
}

if (!pox.begin()) {
Serial.println("FAILED");
oled.clearDisplay();
         oled.setTextSize(1.5);
         oled.setTextColor(1);
         oled.setCursor(0, 0);
         oled.println("Failed");
         oled.display();
for(;;);
} else {
Serial.println("SUCCESS");
 oled.clearDisplay();
         oled.setTextSize(1.5);
         oled.setTextColor(1);
         oled.setCursor(0, 0);
         oled.println("Success");
         oled.display();
         pox.setOnBeatDetectedCallback(onBeatDetected);
}
timeClient.begin();
}

char Time[] = "  :  :  ";
char Date[] = "  -  -20  ";
byte last_second, last_minute, second_, minute_, hour_, wday, day_, month_, year_; 

void loop(){
pox.update();

unsigned long currentMillis=millis();
if(currentMillis - tsLastReport >= interval)
{
pox.shutdown(); //sleep mode

timeClient.update();
    unsigned long unix_epoch = timeClient.getEpochTime();   // get UNIX Epoch time
    second_ = second(unix_epoch);        // get seconds from the UNIX Epoch time
    
      minute_ = minute(unix_epoch);      // get minutes (0 – 59)
      hour_   = hour(unix_epoch);        // get hours   (0 – 23)
      wday    = weekday(unix_epoch);     // get minutes (1 – 7 with Sunday is day 1)
      day_    = day(unix_epoch);         // get month day (1 – 31, depends on month)
      month_  = month(unix_epoch);       // get month (1 – 12 with Jan is month 1)
      year_   = year(unix_epoch) - 2000; // get year with 4 digits – 2000 results 2 digits year (ex: 2018 –> 18)
      Time[7] = second_ % 10 + '0';
      Time[6] = second_ / 10 + '0';
      Time[4] = minute_ % 10 + '0';
      Time[3] = minute_ / 10 + '0';
      Time[1] = hour_   % 10 + '0';
      Time[0] = hour_   / 10 + '0';
      Date[9] = year_   % 10 + '0';
      Date[8] = year_   / 10 + '0';
      Date[4] = month_  % 10 + '0';
      Date[3] = month_  / 10 + '0';
      Date[1] = day_    % 10 + '0';
      Date[0] = day_    / 10 + '0';
      Serial.print(Time);
      String fbtime = String(Time);
      Serial.print(" ");
      display_wday();
      Serial.println(Date);
      String fbdate = String(Date);
      Serial.println(fbtime + " " + fbdate);

      
float bpm = pox.getHeartRate();
int SpO2 =pox.getSpO2();

  Serial.print("Heart rate: ");
  Serial.print(bpm);
  String fireHR = String(bpm);

  Serial.print("bpm  SpO2: ");
  Serial.print(SpO2);
  Serial.println("%");
  String fireSpO2 = String(SpO2);

  Firebase.setString("/Heart rate", fireHR); //đẩy giá trị liên tục nhưng không lưu chuỗi
  Firebase.setString("/SpO2", fireSpO2);
  Firebase.setString("/Store/sp", fireSpO2);
  Firebase.setString("/Store/hr", fireHR);
  Firebase.setString("/Store/time", fbtime +  "  " + fbdate);
  Firebase.pushString("/Storing", "SpO2: " + fireSpO2 + " " + "| " + "Pulse: " + fireHR  +  " | " + fbtime + " | " + fbdate);

  if (Firebase.failed()) //nếu failed thì nên thay đổi fingerprint trong file FirebaseHttpClient.h file
    {
      Serial.print("pushing /logs failed:");
      Serial.println(Firebase.error()); 
      return;
    }

     oled.clearDisplay();
        oled.setTextSize(1.5);
        oled.setTextColor(1);
        oled.setCursor(0,16);
        oled.println(pox.getHeartRate());

        oled.setTextSize(1.5);
        oled.setTextColor(1);
        oled.setCursor(0, 0);
        oled.println("Heart BPM");

        oled.setTextSize(1.5);
        oled.setTextColor(1);
        oled.setCursor(0, 30);
        oled.println("Spo2");

        oled.setTextSize(1.5);
        oled.setTextColor(1);
        oled.setCursor(0, 45);
        oled.println(pox.getSpO2());
        oled.display();


pox.resume(); //resume khi đọc kết quả mới, ready to read new samples
tsLastReport = currentMillis;
}
}
void display_wday()
{
  switch(wday)
  {
    case 1:  
             Serial.print ("SUNDAY "); break;
    case 2:  
             Serial.print ("MONDAY "); break;
    case 3: 
             Serial.print ("TUESDAY "); break;
    case 4:  
             Serial.print ("WEDNESDAY "); break;
    case 5:  
             Serial.print ("THURSDAY "); break;
    case 6:  
             Serial.print ("FRIDAY "); break;
    default: 
             Serial.print ("SATURDAY ");
  }
}
