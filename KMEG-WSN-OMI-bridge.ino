/**
 * OMI_Write.ino
 * NOTE: See also Makefile for configuration documentation!
 */


// LIBRARIES
#include <Arduino.h>
#include <pgmspace.h>   // `F` macro for storing strings to flash (instead of ram)
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
//#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

// included in omi processing (no guard)
//#include <buildinfo.h>      // version info

#include "Config.h"  // See this for configuration
#include "MyCertificates.h" 
#include "HttpsCertificateExtension.h" // Extension to use the easy Http class with client certificates
#include "KMEGLib/SensorNet.h"          // yggdrasil protocol handling
#include "OMI-processing.h"   // makes and sends all O-MI messages

// Used for a hack in the ESP8266HttpClient that enables certificates when creating tcps context

// Get a character from YGGDRASIL serial, (re)implemented for SensorNet.h library (that used linux serial before)
char getch(void) {
    if (YGGDRASIL.available() > 0)
        return YGGDRASIL.read();

    // first small delay if data becomes available
    delay(10);
    if (YGGDRASIL.available() > 0)
        return YGGDRASIL.read();

    delay(100); // bigger delay for wlan activity
    if (YGGDRASIL.available() > 0)
        return YGGDRASIL.read();
    return 0;
}

void putch(unsigned char buf){
   YGGDRASIL.write(buf); 
}



static ESP8266WiFiMulti WiFiMulti;

#ifdef USE_CLIENTCERTIFICATE
static HttpsCertificateClient http;
#else
static HTTPClient http;
#endif
static ESPCertificateUpdate ESPUpdater;

void checkForUpdates() {
    yield();
    t_httpUpdate_return ret = ESPUpdater.update(
        UPDATE_URL,
        String(_BuildInfo.src_version) + "_" + _BuildInfo.time,
        OMI_CERT_FINGERPRINT,
        client_crt, client_crt_len, client_key, client_key_len);

    yield();
    switch(ret) {
        case HTTP_UPDATE_FAILED:
            DBGSTREAM.println("[update] Update failed.");
            break;
        case HTTP_UPDATE_NO_UPDATES:
            DBGSTREAM.println("[update] Update no Update.");
            break;
        case HTTP_UPDATE_OK:
            DBGSTREAM.println("[update] Update ok."); // may not called we reboot the ESP
            break;
    }
}

bool commsTested = false;

void resetArray(uint8_t numV);

// Setup software serial RX D1(5) TX D2(4) moved to Sensornet.cpp
// SoftwareSerial swSer(5, 4, false, 10); 

void setup() {

    DBGSTREAM.begin(115200); // Debugging output at 115200 baud rate
    DBGSTREAM.setDebugOutput(true);
    YGGDRASIL.begin(115200);

#ifdef SWAPSERIAL0
    Serial.swap();
#endif

    DBGSTREAM.println();
    DBGSTREAM.println();
    DBGSTREAM.println();

    for(uint8_t t = 5; t > 0; t--) {
        DBGSTREAM.printf(FS("[SETUP] WAIT %d...\r\n"), t);
        DBGSTREAM.flush();
        delay(1000);
    }
    DBGSTREAM.println();
    DBGSTREAM.println(F("[SETUP] Current Version: "));
    DBGSTREAM.printf(FS("%s\r\n Arduino/ESP version: %s\r\n Build time: %s %s\r\n"),
        _BuildInfo.src_version, _BuildInfo.env_version, _BuildInfo.date, _BuildInfo.time);

    DBGSTREAM.printf(
        FS("[SETUP] Free heap RAM: %u  Program flash: Free (for updating): %u Used: %u \r\n")
        , ESP.getFreeHeap(), ESP.getFreeSketchSpace(), ESP.getSketchSize());

    DBGSTREAM.print(F("[SETUP] Wifi macAddress: "));
    DBGSTREAM.println(WiFi.macAddress());

    // add APs that can be connected to; (ssid, pass)
    WiFiMulti.addAP(FS("aalto open"), NULL);
    //WiFiMulti.addAP("ssid", "pass");
    //WiFi.begin("aalto open", NULL);

    http.setReuse(true);
    
    // Test internet connection

    DBGSTREAM.println(F("[SETUP] Checking for updates..."));
#if UPDATE_ON_BOOT
    checkForUpdates();
#endif

    DBGSTREAM.println(F("[SETUP] Testing O-MI node link by sending Gateway version data"));
    uint8_t wifiCounter;
    for (wifiCounter = 0; wifiCounter < 60; wifiCounter++) {
        if (WiFiMulti.run() == WL_CONNECTED) {
            if (!createBridgeBootOMI()){
                DBGSTREAM.println(F("[SETUP] FATAL: O-MI node link payload creation failed!"));
                delay(1000);
                ESP.restart();
            }
            if (!trySend(http)){
                DBGSTREAM.println(F("[SETUP] O-MI node link sending failed"));
            } else break;
        }
        delay(4000);
    }
    if (wifiCounter >= 60) {
            DBGSTREAM.println(F("[SETUP] O-MI node link sending failed"));
            delay(1000);
            ESP.restart();
    }

    DBGSTREAM.println();
    DBGSTREAM.println(F("[SETUP] O-MI node link ok"));
    DBGSTREAM.println();


    DBGSTREAM.println(F("[SETUP] Testing YGGDRASIL serial comms"));
    uint8_t counter = 0;
    //for(uint16_t t = 600; t > 0; t--) {
    while(1){
        while (YGGDRASIL.available()) {
            char input = YGGDRASIL.read();
            DBGSTREAM.printf("%c:%02x ", input, input);
            ++counter;
        }
        if (counter > 50) break;
        DBGSTREAM.flush();
        delay(100);
    }
    DBGSTREAM.println();
    DBGSTREAM.println(F("[SETUP] YGGDRASIL link ok"));
    DBGSTREAM.println();
    resetArray(20);
}


// GLOBALS
//static NodeStr packetData; // data to send
NodeStr accData[20]; // = (NodeStr*) malloc(20 * sizeof(NodeStr)); //this is array containing the accumulated Data over the interval defined; add lenght to DEFINE instead of magic value
static uint8_t sendRetries=0; // retry counter
unsigned long previousMillis = 0; // last time data was sent
uint8_t numValues = 0; //remember to keep this to array size

void handleIncomingData() {
    if (numValues < 20 && getNode(accData, numValues)) {
        DBGSTREAM.printf(FS("\r\nPacket success. \r\n\r\n"));
        sendRetries = MAX_RETRIES;
    } else {
        DBGSTREAM.printf(",");
    }
}

void sendOK(){
    const char ok[] = "[OK]";
    YGGDRASIL.println(ok);
    delay(512);
    YGGDRASIL.println(ok);
    delay(512);
    YGGDRASIL.println(ok);
    delay(512);
    YGGDRASIL.println(ok);
    delay(512);
    YGGDRASIL.println(ok);
    delay(512);
    YGGDRASIL.println(ok);
}
void resetArray(uint8_t numV) {
    for(int i = 0; i<numV; i++){
        memset(&accData[i], 0, sizeof(accData[i]));
        accData[i].intValues[4] = 0xFFFF; //max value for uint16_t
    }
    numValues = 0;   
}
// TODO: This is done at SensorNet.cpp atm
void loop() {
#if HACK_STRATEGY & HACK_ACTIVE_TX
#warning Connect TX line, that might short stuff if no diodes and pullup present

    DBGSTREAM.println(F("[main] Warning: sending OK"))
    sendOK();

#endif

    // Save incoming data to globals
    handleIncomingData();

    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis >= interval){
      previousMillis = currentMillis;
      if(numValues > 0){
      // Try to send the data in globals
          if ( (sendRetries>0)
            && (WiFiMulti.run() == WL_CONNECTED)) { // Connected to an AP
  
              if (createOMI(accData, numValues) && trySend(http)) {
                  sendRetries = 0; 
              } else {
                  --sendRetries;
              } 
          }
          //reset array
          DBGSTREAM.printf(FS("Resetting data, array length was %u \r\n" ), numValues);
          resetArray(numValues);
      }  
    }

    //delay();
}

