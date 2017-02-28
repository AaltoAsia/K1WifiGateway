#ifndef BRIDGE_CONFIG_H
#define BRIDGE_CONFIG_H

#include <Arduino.h>

// Has build information (buildtime, versions)
#include <buildinfo.h>

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

// NOTE: Not implemented really, active tx is implemented in SensorNet.cpp
#ifndef HACK_STRATEGY
#define HACK_STRATEGY HACK_PASSIVE
#endif

// Whether to check updates on boot
#define UPDATE_ON_BOOT 1

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


const uint16_t interval = 60000; // interval at which to send data( cap is 10 seconds * 30 = 300 seconds for the PIR sensor resolution)

//////////////////////////////////////////////////////////////
// Omi-processing

// How many object can be stored in http buffer for one request
#define MAX_OBJECTS 16

// How many items can be stored in http buffer for one write request
#define MAX_ITEMS_PER_WRITE 16

// How many characters is max size for InfoItem name + value
#define MAX_II_NAME_VALUE_SIZE 40

#define VALUE_LEN 15

//#ifndef OMI_URL

#define OMI_CERT 1

//#endif

#endif
