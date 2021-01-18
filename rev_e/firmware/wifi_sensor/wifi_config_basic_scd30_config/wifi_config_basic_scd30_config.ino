#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <HTTPClient.h>
#include "credentials.h" //local server credentials
#include <ArduinoJson.h>


void setup() {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

    // put your setup code here, to run once:
    Serial.begin(115200);
    
    
    // WiFi.mode(WiFi_STA); // it is a good practice to make sure your code sets wifi mode how you want it.

    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    //reset settings - wipe credentials for testing
    //wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
     res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    //res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }

}

void loop() {
    // put your main code here, to run repeatedly:

    DynamicJsonDocument doc(1024);

doc["deviceId"] =  12;
JsonObject fields = doc.createNestedObject("fields");
fields["bmp_temp"]=12;
fields["bmp_press"]=12;

fields["temp"]=12;
   fields["humid"]=12;
fields["co2"]=12;

String json;
serializeJson(doc, json);
serializeJson(doc, Serial);

    if(WiFi.status()== WL_CONNECTED){

HTTPClient http;
        int httpCode;

 http.begin(bayou_url);
        http.addHeader("Authorization",bayou_privkey);
        http.addHeader("Content-Type", "application/json");

        httpCode = http.PUT(json);

        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.println(httpCode);

          
            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
               Serial.print(payload);

            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
           
        }

        http.end();
        
    }
    delay(5000);
    
    }
    
