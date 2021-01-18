#include <Arduino.h>

#include <WiFi.h>      // Replace with WiFi.h for ESP32
#include <WebServer.h> // Replace with WebServer.h for ESP32
#include <AutoConnect.h>

WebServer webServer; // Replace with WebServer for ESP32
AutoConnect portal(webServer);
AutoConnectConfig acConfig("Test_AC", ""); //no pasword

ACText(mp_header, "Test_AC", "color:red;font-family:verdana;font-size:300%;");
ACSubmit(action, "Action please !", "/action");
AutoConnectAux mainPage("/", "Test_AC", true, {mp_header, action});



void deleteAllCredentials(void) {
    AutoConnectCredential credential;
    station_config_t config;
    uint8_t ent = credential.entries();

    
    Serial.printf("credentials.entries = %d\n", ent);

    while (ent--) {
        credential.load((int8_t)0, &config);
        credential.del((const char*)&config.ssid[0]);
    }
}

void onAction()
{
    Serial.println("Action button was pressed");

    Serial.println("Deleting credentials...:");
    deleteAllCredentials();


    char content[] = "Action button was pressed; ESP32 is restarting...";
    webServer.send(200, "text/plain", content);

    // Clear WiFi connection, not AutoConnectCredential
    WiFi.disconnect(false, true); //WiFi.disconnect(bool wifioff, bool eraseap)

    ESP.restart();
}


void setup()
{
    // put your setup code here, to run once:

    Serial.begin(115200);
    Serial.println("Started ....");

    acConfig.title = "My menu";
    acConfig.autoReconnect = true; //Tries to connect to any of the saved SSIDs

    portal.config(acConfig);

    portal.join({mainPage});

    webServer.on("/action", onAction);

    if (portal.begin())
    {
        Serial.println("WiFi connected: " + WiFi.localIP().toString());
    }
}

void loop()
{
    // put your main code here, to run repeatedly:

    portal.handleClient();

}
