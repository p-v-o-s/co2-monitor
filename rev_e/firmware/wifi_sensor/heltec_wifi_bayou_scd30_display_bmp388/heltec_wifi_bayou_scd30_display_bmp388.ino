/**
 * BasicHTTPClient.ino
 *
 *  Created on: 24.05.2015
 *
 */
 #include <U8x8lib.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "credentials.h" // My WiFi configuration.

#include <HTTPClient.h>

#define USE_SERIAL Serial
#include <ArduinoJson.h>

WiFiMulti wifiMulti;

#include <ArduinoJson.h> //https://arduinojson.org/v6/doc/installation/

#include <Wire.h>


#include <BMP388_DEV.h>                           // Include the BMP388_DEV.h library
float bmp_temperature, bmp_pressure, bmp_altitude;            // Create the temperature, pressure and altitude variables
BMP388_DEV bmp388; 

#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
SCD30 airSensor;

#define LED 13

unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 90 seconds (5000)
unsigned long timerDelay = 90000;

#define sensorID 15

// for max4466
const int sampleWindow = 10000; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

float mic_level;

#define delayTime 300000

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);


void setup() {

u8x8.begin();
  //u8x8.setFont(u8x8_font_chroma48medium8_r);
//u8x8.setFont(u8x8_font_8x13B_1x2_f);
u8x8.setFont(u8x8_font_7x14B_1x2_f);

  u8x8.drawString(0, 0, "Gateway on!");
  
    Wire.begin();

bmp388.begin();                                 // Default initialisation, place the BMP388 into SLEEP_MODE 
  bmp388.setTimeStandby(TIME_STANDBY_1280MS);     // Set the standby time to 1.3 seconds
  bmp388.startNormalConversion();                 // Start BMP388 continuous conversion in NORMAL_MODE  


  if (airSensor.begin() == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1)
      ;
  }


     pinMode(LED, OUTPUT);
     
    USE_SERIAL.begin(9600);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

wifiMulti.addAP(SSID,WiFiPassword);



}

void loop() {


while (!bmp388.getMeasurements(bmp_temperature, bmp_pressure, bmp_altitude))    // Check if the measurement is complete
  {
    Serial.println("getting BMP388 measurements ..");
    delay(500);
  }

float bmp_temp = roundf(bmp_temperature * 100) / 100;
float bmp_press = roundf(bmp_pressure * 100) / 100;

  DynamicJsonDocument doc(1024);

doc["deviceId"] =  sensorID;
JsonObject fields = doc.createNestedObject("fields");
fields["bmp_temp"]=bmp_temp;
fields["bmp_press"]=bmp_press;



if (airSensor.dataAvailable())
  {

  
    int co2 = airSensor.getCO2();
    float temp = roundf(airSensor.getTemperature()* 100) / 100;
    float humid = roundf(airSensor.getHumidity()* 100) / 100;
    
    Serial.print("co2(ppm):");
    Serial.print(co2);

    Serial.print(" temp(C):");
    Serial.print(temp, 1);

    Serial.print(" humidity(%):");
    Serial.println(humid, 1);

    Serial.println();
    Serial.println("collecting mic sample ...");

        fields["temp"]=temp;
   fields["humid"]=humid;
fields["co2"]=co2;

  /*
      //do a mic sample
    unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level

   unsigned int signalMax = 0;
   unsigned int signalMin = 4096;


   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(0);
      if (sample < 4096)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   double volts = (peakToPeak * 3.3) / 4096;  // convert to volts

mic_level = roundf(volts* 100) / 100;

    Serial.print("Mic level was: ");
    Serial.print(volts);
    Serial.println(" Volts");

   
fields["mic"]=mic_level;
*/

String json;
serializeJson(doc, json);
serializeJson(doc, Serial);

u8x8.clear();

u8x8.setFont(u8x8_font_7x14B_1x2_f);

u8x8.setCursor(0,0);
u8x8.print("CO2: ");
u8x8.print(co2);

u8x8.setCursor(0,2);
u8x8.print("Temp: ");
u8x8.print(temp);

u8x8.setCursor(0,4);
u8x8.print("Humid: ");
u8x8.print(humid);

u8x8.setCursor(0,6);
u8x8.print("Press: ");
u8x8.print(bmp_press);



// now send via wifi
// wait for WiFi connection
    if((wifiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;
        int httpCode;

// BAYOU -----------------------------------------------

        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        USE_SERIAL.print(bayou_url);
        
        http.begin(bayou_url);
        http.addHeader("Authorization",bayou_privkey);

        //http.begin(url);
        //http.addHeader(authorize);
        
        http.addHeader("Content-Type", "application/json");
        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        
      
        httpCode = http.PUT(json);        

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            u8x8.setFont(u8x8_font_chroma48medium8_r);
u8x8.setCursor(13,0);
u8x8.print(httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
               

            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            u8x8.setFont(u8x8_font_chroma48medium8_r);
u8x8.setCursor(13,0);
u8x8.print(http.errorToString(httpCode).c_str());
        }

        http.end();
        
        
    } // end if wifi connected
  } // end if air sensor meas avail

/*
u8x8.setFont(u8x8_font_chroma48medium8_r);
u8x8.setCursor(11,0);
u8x8.print("POST");
*/

    delay(delayTime); // wait a minute until the next post
      
} // end loop
