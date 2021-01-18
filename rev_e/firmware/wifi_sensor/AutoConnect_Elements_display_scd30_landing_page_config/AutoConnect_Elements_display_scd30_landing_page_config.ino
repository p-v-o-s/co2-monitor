/*
  Elements.ino, Example for the AutoConnect library.
  Copyright (c) 2019, Hieromon Ikasamo
  https://github.com/Hieromon/AutoConnect
  This software is released under the MIT License.
  https://opensource.org/licenses/MIT

  This example demonstrates the typical behavior of AutoConnectElement.
  It also represents a basic structural frame for saving and reusing
  values ​​entered in a custom web page into flash.
*/

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
using WebServerClass = ESP8266WebServer;
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
using WebServerClass = WebServer;
#endif
#include <FS.h>
#include <AutoConnect.h>
 #include <U8x8lib.h>
 #include "credentials.h" //local server credentials
#include <ArduinoJson.h>


#define PARAM_FILE      "/elements.json"

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);


char put_url [200];
char drive_url [200];
char csv_url [200];

static const char PAGE_ELEMENTS[] PROGMEM = R"(
{
  "uri": "/elements",
  "title": "Elements",
  "menu": true,
  "element": [
    {
      "name": "text",
      "type": "ACText",
      "value": "AutoConnect element behaviors collection",
      "style": "font-family:Arial;font-size:18px;font-weight:400;color:#191970"
    },
    {
      "name": "check",
      "type": "ACCheckbox",
      "value": "check",
      "label": "Check",
      "labelposition": "infront",
      "checked": true
    },
    {
      "name": "input",
      "type": "ACInput",
      "global": true,
      "label": "Text input",
      "placeholder": "This area accepts hostname patterns",
      "pattern": "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$"
    },
    {
      "name": "radio",
      "type": "ACRadio",
      "value": [
        "Button-1",
        "Button-2",
        "Button-3"
      ],
      "label": "Radio buttons",
      "arrange": "vertical",
      "checked": 1
    },
    {
      "name": "select",
      "type": "ACSelect",
      "option": [
        "Option-1",
        "Option-2",
        "Option-3"
      ],
      "label": "Select",
      "selected": 2
    },
    {
      "name": "load",
      "type": "ACSubmit",
      "value": "Load",
      "uri": "/elements"
    },
    {
      "name": "save",
      "type": "ACSubmit",
      "value": "Save",
      "uri": "/save"
    },
    {
      "name": "adjust_width",
      "type": "ACElement",
      "value": "<script type=\"text/javascript\">window.onload=function(){var t=document.querySelectorAll(\"input[type='text']\");for(i=0;i<t.length;i++){var e=t[i].getAttribute(\"placeholder\");e&&t[i].setAttribute(\"size\",e.length*.8)}};</script>"
    }
  ]
}
)";

static const char PAGE_SAVE[] PROGMEM = R"(
{
  "uri": "/save",
  "title": "Elements",
  "menu": false,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "format": "Elements have been saved to %s",
      "style": "font-family:Arial;font-size:18px;font-weight:400;color:#191970"
    },
    {
      "name": "validated",
      "type": "ACText",
      "style": "color:red"
    },
    {
      "name": "echo",
      "type": "ACText",
      "style": "font-family:monospace;font-size:small;white-space:pre;"
    },
    {
      "name": "ok",
      "type": "ACSubmit",
      "value": "OK",
      "uri": "/elements"
    }
  ]
}
)";

WebServerClass  server;
AutoConnect portal(server);
AutoConnectConfig config;
AutoConnectAux  elementsAux;
AutoConnectAux  saveAux;

void handleRoot() {
  String page = PSTR(
"<html>"
"<head>"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "<style type=\"text/css\">"
    "body {"
    "-webkit-appearance:none;"
    "-moz-appearance:none;"
    "font-family:'Arial',sans-serif;"
    "text-align:center;"
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
  "User guide: <a href=\"http://pvos.org\">pvos.org/co2</a><br>"
  //"<h3>Data & Graphs</h3>");
  "<br><br>");

// bayou graphs / drive landing page 
page+= String(F("<b>Feed:</b> <a href=\""));
  page+= String(drive_url);
  page+= String(F("\">"));
   page+= String(bayou_feed);
   page+=String(F("</a><br><br>"));

  // csv landing page
  page+= String(F("<b>Data:</b> <a href=\""));
   page+= String(csv_url);
  page+= String(F("\">CSV</a><br>"));

  
/*
  page += String(F("<h3>Actions:<h3>"));
  page += String(F("<p><a class=\"button\" href=\"/io?v=low\">calibrate</a><a class=\"button\" href=\"/io?v=high\">configure</a></p>"));
  */
  
  page += String(F("</body></html>"));
  portal.host().send(200, "text/html", page);
}


void setup() {

sprintf (csv_url, "%s%s%s%s", bayou_base_url, "/api/drives/", bayou_feed, "/csv");
sprintf (drive_url, "%s%s%s", bayou_base_url, "/drives/", bayou_feed);
sprintf (put_url, "%s%s%s", bayou_base_url, "/api/drives/", bayou_feed);

  u8x8.begin();
  u8x8.setFont(u8x8_font_7x14B_1x2_f);
 u8x8.clear();
 u8x8.setCursor(0,0); 
 u8x8.print("_CONFIGURE_");
 u8x8.setCursor(0,2); 
u8x8.print("AP:esp32ap");
 u8x8.setCursor(0,4); 
u8x8.print("pw:");
u8x8.print("12345678");

  delay(1000);
  Serial.begin(115200);
  Serial.println();

  // Responder of root page handled directly from WebServer class.
  /*
  server.on("/", []() {
    String content = "PVOS CO2 sensor.&ensp;";
    content += AUTOCONNECT_LINK(COG_24);
    server.send(200, "text/html", content);
    
  });
  */

  server.on("/", handleRoot);
  

  // Load a custom web page described in JSON as PAGE_ELEMENT and
  // register a handler. This handler will be invoked from
  // AutoConnectSubmit named the Load defined on the same page.
  elementsAux.load(FPSTR(PAGE_ELEMENTS));
  elementsAux.on([] (AutoConnectAux& aux, PageArgument& arg) {
    if (portal.where() == "/elements") {
      // Use the AutoConnect::where function to identify the referer.
      // Since this handler only supports AutoConnectSubmit called the
      // Load, it uses the uri of the custom web page placed to
      // determine whether the Load was called me or not.
      SPIFFS.begin();
      File param = SPIFFS.open(PARAM_FILE, "r");
      if (param) {
        aux.loadElement(param, { "text", "check", "input", "radio", "select" } );
        param.close();
      }
      SPIFFS.end();
    }
    return String();
  });

  saveAux.load(FPSTR(PAGE_SAVE));
  saveAux.on([] (AutoConnectAux& aux, PageArgument& arg) {
    // You can validate input values ​​before saving with
    // AutoConnectInput::isValid function.
    // Verification is using performed regular expression set in the
    // pattern attribute in advance.
    AutoConnectInput& input = elementsAux["input"].as<AutoConnectInput>();
    aux["validated"].value = input.isValid() ? String() : String("Input data pattern missmatched.");

    // The following line sets only the value, but it is HTMLified as
    // formatted text using the format attribute.
    aux["caption"].value = PARAM_FILE;

#if defined(ARDUINO_ARCH_ESP8266)
    SPIFFS.begin();
#elif defined(ARDUINO_ARCH_ESP32)
    SPIFFS.begin(true);
#endif
    File param = SPIFFS.open(PARAM_FILE, "w");
    if (param) {
      // Save as a loadable set for parameters.
      elementsAux.saveElement(param, { "text", "check", "input", "radio", "select" });
      param.close();
      // Read the saved elements again to display.
      param = SPIFFS.open(PARAM_FILE, "r");
      aux["echo"].value = param.readString();
      param.close();
    }
    else {
      aux["echo"].value = "SPIFFS failed to open.";
    }
    SPIFFS.end();
    return String();
  });
  
  

  portal.join({ elementsAux, saveAux});

  
  config.ticker = true;
  portal.config(config);
    if (portal.begin()) {

      u8x8.clear();
u8x8.setFont(u8x8_font_7x14B_1x2_f);
u8x8.setCursor(0,0); 
 u8x8.print("Configured!");
u8x8.setCursor(0,2);
u8x8.print("ap*: ");
u8x8.print(WiFi.SSID());
u8x8.setCursor(0,4);
u8x8.print("ip: ");
u8x8.print(WiFi.localIP().toString());
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }
}

void loop() {
  portal.handleClient();

  DynamicJsonDocument doc(1024);

doc["deviceId"] =  10;
JsonObject fields = doc.createNestedObject("fields");
fields["bmp_temp"]=10;
fields["bmp_press"]=10;

fields["temp"]=10;
   fields["humid"]=10;
fields["co2"]=10;

String json;
serializeJson(doc, json);
serializeJson(doc, Serial);


      u8x8.clear();
u8x8.setFont(u8x8_font_7x14B_1x2_f);
u8x8.setCursor(0,0); 
 u8x8.print("Configured!");
u8x8.setCursor(0,2);
u8x8.print("ap*: ");
u8x8.print(WiFi.SSID());
u8x8.setCursor(0,4);
u8x8.print("ip: ");
u8x8.print(WiFi.localIP().toString());
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    u8x8.setCursor(0,6);
u8x8.print("co2: ");
u8x8.print(23);    


if(WiFi.status()== WL_CONNECTED){

Serial.println(put_url);

HTTPClient http;
        int httpCode;

 http.begin(put_url);
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
