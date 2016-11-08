/**
 * OMI_Write.ino
 * NOTE: See also Makefile for configuration documentation!
 */

// Select hacking strategy, combination of:

// * Passive listen only, requires that the wifi chip connects properly and sends proper answers
#define HACK_PASSIVE 0

// * Sends [OK] answers through the tx line of wifi chip, actively tries to flood them to the mcu
#define HACK_ACTIVE_TX 1

// * Sends GPIO command to set data/command-mode pin (31) to zero
#define HACK_DATA_GPIO0 2
// * Sends GPIO command to set data/command-mode pin (31) to one
#define HACK_DATA_GPIO1 4

// * Sends configuration to disable HW trigger for changing data/command mode (default: GPIO29,pin46)
#define HACK_DISABLE_HW_MODE_TRIGGER 8

// NOTE: Not implemented really
#ifndef HACK_STRATEGY
#define HACK_STRATEGY HACK_PASSIVE
#endif

// Fetches updates from this
#define UPDATE_URL FS("https://otaniemi3d.cs.hut.fi/k1updates")
// O-MI Node for write requests
#define OMI_URL FS("https://otaniemi3d.cs.hut.fi/omi/node/")

// SHA1 Fingerprint, can be copied from browser, click the lock next to url bar and view certificate
// NOTE: Probably needs updating if certificate updates
#define OMI_CERT_FINGERPRINT String("6D 7F AE 98 E6 4A 74 76 45 26 7F 66 14 3C 9F 58 43 CB 09 B5")

// Retries for sending if http fails
#define MAX_RETRIES 1

// Loads the client certificate from SPIFFS files: /client.crt and /client.key
// WARNING: Requires modified ESP8266HttpClient currently (2016-04-08)
#define USE_CLIENTCERTIFICATE 1


// Serial for debugging, connect esp TX to usb serial RX
#ifndef DBGSTREAM
#define DBGSTREAM Serial1
#endif
// Input for Yggdrasil serial comms, connect this esp RX to TX (marked) pin on bridge pcb
#ifndef YGGDRASIL
#define YGGDRASIL Serial
#endif


// LIBRARIES
#include <Arduino.h>
#include <pgmspace.h>   // `F` macro for storing strings to flash (instead of ram)
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
//#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include <buildinfo.h>      // version info

#include "HttpsCertificateExtension.h" // Extension to use the easy Http class with client certificates
#include "MyCertificates.h"     // contains unencrypted client private key and certifiacate in DER format
#include "SensorNet.h"          // yggdrasil protocol handling
#include "OMI-processing.cpp"   // makes and sends all O-MI messages

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
    t_httpUpdate_return ret = ESPUpdater.update(
        UPDATE_URL,
        String(_BuildInfo.src_version) + "_" + _BuildInfo.time,
        OMI_CERT_FINGERPRINT,
        client_crt, client_crt_len, client_key, client_key_len);

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
    DBGSTREAM.println(F("[SETUP] Checking for updates..."));
    checkForUpdates();

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
const uint16_t interval = 20000; // interval at which to send data
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
        memset(accData[i].intValues, 0, 5);
        accData[i].Id = 0;
        accData[i].Last_seq = 0;
        accData[i].length = 0;
        accData[i].humCount = 0;
        accData[i].tempCount = 0;
        accData[i].lumCount = 0;
        accData[i].co2Count = 0;
        accData[i].pirCount = 0;
        accData[i].Rssi = 0;
        accData[i].packetLost = false;
        accData[i].Ack = false;
    }
    numValues = 0;   
}
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

