e/*
  ESP8266/ESP32 publish the RSSI as the WiFi signal strength to ThingSpeak channel.
  This example is for explaining how to use the AutoConnect library.

  In order to execute this example, the ThingSpeak account is needed. Sing up
  for New User Account and create a New Channel via My Channels.
  For details, please refer to the project page.
  https://hieromon.github.io/AutoConnect/howtoembed.html#used-with-mqtt-as-a-client-application

  Also, this example uses AutoConnectAux menu customization which stored in SPIFFS.
  To evaluate this example, you upload the contents as mqtt_setting.json of
  the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu
  in Arduino IDE).

  This example is based on the thinkspeak.com environment as of Dec. 20, 2018.
  Copyright (c) 2018 Hieromon Ikasamo.
  This software is released under the MIT License.
  https://opensource.org/licenses/MIT
*/

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#define GET_CHIPID()  (ESP.getChipId())
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#define GET_CHIPID()  ((uint16_t)(ESP.getEfuseMac()>>32))
#endif
#include <FS.h>
//#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient
#include <AutoConnect.h>
 #include <U8x8lib.h>
#include <Bounce2.h> // https://github.com/thomasfredericks/Bounce2

String bayou_base_url="data.pvos.org/co2/data/";
String bayou_feedkey = "175000f8eaac54d3361a3f56832074dd3fb5ac18fcfd5672";
String bayou_writekey = "93c7fd0f9175c1a149414842f2af239eac38f32d171540f5";
String post_url ="";

int co2 = 410; // initial (dummy) value
int forcePPM = 410; // force calibration value

#include <Wire.h>
#include <BMP388_DEV.h>                           // Include the BMP388_DEV.h library
float bmp_temperature, bmp_pressure, bmp_altitude;            // Create the temperature, pressure and altitude variables
BMP388_DEV bmp388; 
#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
SCD30 airSensor;


#define PARAM_FILE      "/param.json"
#define AUX_MQTTSETTING "/mqtt_setting"
#define AUX_MQTTSAVE    "/mqtt_save"
#define AUX_MQTTCLEAR   "/mqtt_clear"

typedef WebServer WiFiWebServer;


AutoConnect  portal;
AutoConnectConfig config;
WiFiClient   wifiClient;
//PubSubClient mqttClient(wifiClient);
String  serverName;
String  channelId;
String  userKey;
String  apiKey;
String  apid;
String  hostName;
String upd;
unsigned int  updateInterval = 0;
unsigned long lastPub = 0;

#define BUTTON_A_PIN 37 
#define BUTTON_B_PIN 36
#define LED_PIN 25

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

// INSTANTIATE A Button OBJECT
Button button_A = Button();
Button button_B = Button();
// SET A VARIABLE TO STORE THE LED STATE
int ledState = LOW;

long lastMeasureTime = 0;  // the last time the output pin was toggled

long measureDelay = 10000;

int firstLoop = 1;

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


String loadParams(AutoConnectAux& aux, PageArgument& args) {
  (void)(args);
  Serial.println("loading...");
  File param = SPIFFS.open(PARAM_FILE, "r");
  if (param) {
    aux.loadElement(param);
    //aux.loadElement(param, { "mqttserver", "channelid", "userkey", "apikey"} );

Serial.println("loaded new params ...");

   bayou_base_url=aux["mqttserver"].value;
   bayou_feedkey = aux["userkey"].value;
   bayou_writekey = aux["apikey"].value;
   
   post_url="http://"+bayou_base_url+"/"+bayou_feedkey;
   
   upd = aux["period"].value;
   Serial.println(upd);
  measureDelay = upd.substring(0, 2).toInt() * 60000;
  
    param.close();
  }
  else
    Serial.println(PARAM_FILE " open failed");
  return String("");
}

String saveParams(AutoConnectAux& aux, PageArgument& args) {
  serverName = args.arg("mqttserver");
  serverName.trim();
  Serial.println(serverName);
  bayou_base_url = serverName;

  channelId = args.arg("channelid");
  channelId.trim();
  
  userKey = args.arg("userkey");
  userKey.trim();
  Serial.println(userKey);
  bayou_feedkey = userKey;
  
  apiKey = args.arg("apikey");
  apiKey.trim();
  Serial.println(apiKey);
  bayou_writekey = apiKey;
  
  upd = args.arg("period");
  measureDelay = upd.substring(0, 2).toInt() * 60000;

  Serial.println(measureDelay); 
  
  String uniqueid = args.arg("uniqueid");

  hostName = args.arg("hostname");
  hostName.trim();

  
  post_url="http://"+bayou_base_url+"/"+bayou_feedkey;


  firstLoop = 1; //we should post immediately after saving new params


  // The entered value is owned by AutoConnectAux of /mqtt_setting.
  // To retrieve the elements of /mqtt_setting, it is necessary to get
  // the AutoConnectAux object of /mqtt_setting.
  File param = SPIFFS.open(PARAM_FILE, "w");
  portal.aux("/mqtt_setting")->saveElement(param, { "mqttserver", "channelid", "userkey", "apikey", "period", "uniqueid", "hostname" });
  param.close();

  // Echo back saved parameters to AutoConnectAux page.
  AutoConnectText&  echo = aux["parameters"].as<AutoConnectText>();
  echo.value = "Server: " + serverName + "<br>";
  echo.value += "Channel ID: " + channelId + "<br>";
  echo.value += "User Key: " + userKey + "<br>";
  echo.value += "API Key: " + apiKey + "<br>";
  echo.value += "Update period: " + String(updateInterval / 60000) + " min<br>";
  echo.value += "Use APID unique: " + uniqueid + "<br>";
  echo.value += "ESP host name: " + hostName + "<br>";

  return String("");
}

void handleRoot() {
  String  content = PSTR(
    "<style type=\"text/css\">"
    "body {"
    "-webkit-appearance:none;"
    "-moz-appearance:none;"
    "font-family:'Arial',sans-serif;"
    "text-align:left;"
    "}"
    ".menu > a:link {"
    "position: absolute;"
    "display: inline-block;"
    "right: 12px;"
    "padding: 0 6px;"
    "text-decoration: none;"
    "}"
    ".button {"
    "display:inline-block;"
    "border-radius:7px;"
    //"background:#73ad21;"
    "background:green;"
    "margin:0 10px 0 10px;"
    "padding:10px 20px 10px 20px;"
    "text-decoration:none;"
    "color:#000000;"
    "}"
  "</style>"
"</head>"
"<body>"
  "<div class=\"menu\">" AUTOCONNECT_LINK(BAR_32) "</div>"
  "<h1>CO2 Sensor</h1>"
  "Firmware version <a href=\"https://github.com/edgecollective/co2-remote-and-gateway/tree/rev_e_wifi_config/rev_e/firmware/wifi_sensor/AutoConnect_Elements_display_scd30_landing_page\">0.2</a><br>"
  "User guide: <a href=\"http://pvos.org\">pvos.org/co2</a><br>");
  content += String(F("<p><a class=\"button\" href=\"/_ac/config\">WiFi Settings</a><a class=\"button\" href=\"/mqtt_setting\">Bayou Settings</a></p>"));
  
  content += String(F("</body></html>"));
  WiFiWebServer&  webServer = portal.host();
  webServer.send(200, "text/html", content);
}


// Load AutoConnectAux JSON from SPIFFS.
bool loadAux(const String auxName) {
  bool  rc = false;
  String  fn = auxName + ".json";
  File fs = SPIFFS.open(fn.c_str(), "r");
  if (fs) {
    rc = portal.load(fs);
    fs.close();
  }
  else
    Serial.println("SPIFFS open failed: " + fn);
  return rc;
}

void setup() {


  button_A.attach( BUTTON_A_PIN, INPUT ); // USE EXTERNAL PULL-UP
  button_A.interval(5); 
  button_A.setPressedState(LOW);

    button_B.attach( BUTTON_B_PIN, INPUT ); // USE EXTERNAL PULL-UP
  button_B.interval(5); 
  button_B.setPressedState(LOW);
  
 pinMode(LED_PIN,OUTPUT);
 digitalWrite(LED_PIN,ledState);

 u8x8.begin();
  u8x8.setFont(u8x8_font_7x14B_1x2_f);
 u8x8.clear();
 u8x8.setCursor(0,0); 
 u8x8.print("Configuring...");
 u8x8.setCursor(0,2); 
u8x8.print("AP:esp32ap");
 u8x8.setCursor(0,4); 
u8x8.print("pw:");
u8x8.print("12345678");

  delay(1000);
  Serial.begin(115200);
  Serial.println();


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

  Serial.println(ESP.getFreeHeap(),DEC);
  SPIFFS.begin();

  loadAux(AUX_MQTTSETTING);
  loadAux(AUX_MQTTSAVE);

  AutoConnectAux* setting = portal.aux(AUX_MQTTSETTING);

  //change the menu name on the _ac landing page
  config.title = "CO2 Sensor: WiFi Settings";
  
  if (setting) {
    PageArgument  args;
    AutoConnectAux& mqtt_setting = *setting;
    loadParams(mqtt_setting, args);
    AutoConnectCheckbox&  uniqueidElm = mqtt_setting["uniqueid"].as<AutoConnectCheckbox>();
    AutoConnectInput&     hostnameElm = mqtt_setting["hostname"].as<AutoConnectInput>();
    if (uniqueidElm.checked) {
      config.apid = String("ESP") + "-" + String(GET_CHIPID(), HEX);
      Serial.println("apid set to " + config.apid);
    }
    if (hostnameElm.value.length()) {
      config.hostName = hostnameElm.value;
      Serial.println("hostname set to " + config.hostName);
    }
    config.homeUri = "/";
    portal.config(config);

    portal.on(AUX_MQTTSETTING, loadParams);
    portal.on(AUX_MQTTSAVE, saveParams);
  }
  else
    Serial.println("aux. load error");

  Serial.print("WiFi ");
  if (portal.begin()) {
    config.bootUri = AC_ONBOOTURI_HOME;

       u8x8.clear();
u8x8.setFont(u8x8_font_7x14B_1x2_f);
u8x8.setCursor(0,0); 
 u8x8.print("Configured!");
u8x8.setCursor(0,4);
//u8x8.print("AP:");
u8x8.print(WiFi.SSID());
u8x8.setCursor(0,6);
//u8x8.print("ip: ");
u8x8.print(WiFi.localIP().toString());


    Serial.println("connected:" + WiFi.SSID());
    Serial.println("IP:" + WiFi.localIP().toString());
  } else {
    Serial.println("connection failed:" + String(WiFi.status()));
    while (1) {
      delay(100);
      yield();
    }
  }

  WiFiWebServer&  webServer = portal.host();
  webServer.on("/", handleRoot);

  // remove this once things are working
  measureDelay = 300000;
  
}


int didJustPress=0;

void loop() {


  portal.handleClient();

   button_A.update();
  button_B.update();
 
 if ( button_A.pressed() ) {
    
    // TOGGLE THE LED STATE : 
    ledState = !ledState; // SET ledState TO THE OPPOSITE OF ledState
    //digitalWrite(LED_PIN,HIGH); // WRITE THE NEW ledState

    //display config information:
    
 u8x8.clear();
u8x8.setFont(u8x8_font_7x14B_1x2_f);
u8x8.setCursor(0,0); 
 u8x8.print("Network Config:");
u8x8.setCursor(0,4);
//u8x8.print("AP: ");
u8x8.print(WiFi.SSID());
u8x8.setCursor(0,6);
//u8x8.print("ip: ");
u8x8.print(WiFi.localIP().toString());
    Serial.println("WiFi connected: " + WiFi.localIP().toString());  
 didJustPress = 1;

delay(3000);

// now display usual stuff

didJustPress = 0;

//display usual info

ledState = !ledState;

u8x8.clear();
//u8x8.setFont(u8x8_font_7x14B_1x2_f);
u8x8.setFont(u8x8_font_inr33_3x6_f);
u8x8.setCursor(0,0); 
u8x8.print(co2);

  }

  if ( button_B.pressed() ) {
    
    // TOGGLE THE LED STATE : 
    ledState = !ledState; // SET ledState TO THE OPPOSITE OF ledState
    //digitalWrite(LED_PIN,HIGH); // WRITE THE NEW ledState

    //display info:
    

// set ourselves up to make an immediate measurement
firstLoop = 1;

long pressTime = millis();

int pressCount = 5;

u8x8.clear();
u8x8.setFont(u8x8_font_7x14B_1x2_f);
u8x8.setCursor(0,0); 
 u8x8.print("Calibrating to");
 u8x8.setCursor(0,2); 
 u8x8.print(forcePPM);
 u8x8.print(" ppm");

 int buttonB_state = digitalRead(BUTTON_B_PIN);
 
while (((millis() - pressTime) < 5000) && (buttonB_state==0)) {

   buttonB_state = digitalRead(BUTTON_B_PIN);
  u8x8.setCursor(0,4);
  u8x8.print("in ");
  u8x8.print(pressCount);
  u8x8.print("...");
  pressCount--;
  delay(1000);
}
// now display usual stuff

//didJustPress = 0;

//display usual info
if (pressCount==0) {



// start reading every min and display and ask if it settles 

  u8x8.clear();
u8x8.setFont(u8x8_font_7x14B_1x2_f);
//u8x8.setFont(u8x8_font_inr33_3x6_f);
u8x8.setCursor(0,0); 
 u8x8.print("When CO2 settles,");
 u8x8.setCursor(0,2); 
 u8x8.print("release button");
 u8x8.setCursor(0,4); 
 u8x8.print("to calibrate.");
 delay(2000);

 buttonB_state = digitalRead(BUTTON_B_PIN);
 
while ( buttonB_state==0) {

buttonB_state = digitalRead(BUTTON_B_PIN);

if (airSensor.dataAvailable()) {
co2 = airSensor.getCO2();
u8x8.clear();
//u8x8.setFont(u8x8_font_7x14B_1x2_f);
u8x8.setFont(u8x8_font_inr33_3x6_f);
u8x8.setCursor(0,0); 
u8x8.print(co2);

}

}



airSensor.setForcedRecalibrationFactor(forcePPM);
u8x8.clear();
u8x8.setFont(u8x8_font_7x14B_1x2_f);
//u8x8.setFont(u8x8_font_inr33_3x6_f);
u8x8.setCursor(0,0); 
u8x8.print("Calibrated to:");
u8x8.setCursor(0,2); 
u8x8.print(forcePPM);
u8x8.print(" PPM!");
delay(3000);

}



else {
  u8x8.clear();
u8x8.setFont(u8x8_font_7x14B_1x2_f);
//u8x8.setFont(u8x8_font_inr33_3x6_f);
u8x8.setCursor(0,0); 
 u8x8.print("Force");
 u8x8.setCursor(0,2); 
 u8x8.print("calibration");
 u8x8.setCursor(0,4); 
 u8x8.print("canceled.");
 delay(3000);
}



  }

  
  
  if (  ( (millis() - lastMeasureTime) > measureDelay) || firstLoop) {

Serial.print("getFreeHeap(): ");
Serial.println(ESP.getFreeHeap(),DEC);
  
  if (firstLoop) firstLoop = 0;

if (airSensor.dataAvailable())
  {

    co2 = airSensor.getCO2();
    float temp = roundf(airSensor.getTemperature()* 100) / 100;
    float humid = roundf(airSensor.getHumidity()* 100) / 100;

    while (!bmp388.getMeasurements(bmp_temperature, bmp_pressure, bmp_altitude))    // Check if the measurement is complete
  {
    Serial.println("getting BMP388 measurements ..");
    delay(500);
  }

float bmp_temp = roundf(bmp_temperature * 100) / 100;
float bmp_press = roundf(bmp_pressure * 100) / 100;

  
 u8x8.clear();
//u8x8.setFont(u8x8_font_7x14B_1x2_f);
u8x8.setFont(u8x8_font_inr33_3x6_f);
u8x8.setCursor(0,0); 
u8x8.print(co2);

if(WiFi.status()== WL_CONNECTED){

DynamicJsonDocument doc(1024);

Serial.println(post_url);

doc["private_key"] = bayou_writekey;
doc["co2"] =  co2;
//JsonObject fields = doc.createNestedObject("fields");
doc["tempC"]=temp;
doc["humidity"]=humid;
doc["mic"]=0.;
doc["auxPressure"]=bmp_press;
doc["auxTempC"]=bmp_temp;
doc["aux001"]=0.;
doc["aux002"]=0.;

String json;
serializeJson(doc, json);
serializeJson(doc, Serial);

Serial.println(post_url);

HTTPClient http;
        int httpCode;

 http.begin(post_url);
        //http.addHeader("Authorization",bayou_privkey);
        http.addHeader("Content-Type", "application/json");

        httpCode = http.POST(json);

        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.println(httpCode);
           
          
            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
               Serial.println(payload);

            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
           
        }

        http.end();
}
  }
        lastMeasureTime = millis(); //set the current time

  }
}
