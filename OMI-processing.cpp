
#include "K1Building.h" // constants and metadata
#include "SensorNet.h"  // yggdrasil protocol handling
#include <ESP8266WiFi.h> // For mac address name wifi bridge object

// How many object can be stored in http buffer for one request
#define MAX_OBJECTS 1

// How many items can be stored in http buffer for one write request
#define MAX_ITEMS_PER_WRITE 6

// How many characters is max size for InfoItem name + value
#define MAX_II_NAME_VALUE_SIZE 40

#define VALUE_LEN 15

//#ifndef OMI_URL

#define OMI_CERT 1

//#endif


//enum TrySendRet {
//    Success,
//    NoWifi,
//    Fail
//};


// templates (format string) for sprintf to insert the value
// It has placeholder for id, name and value
static const char writeHeader[] =
"<omi:omiEnvelope xmlns:omi=\"omi.xsd\" version=\"1.0\" ttl=\"20\">"
  "<omi:write msgformat=\"odf\">"
    "<omi:msg>"
      "<Objects xmlns=\"odf.xsd\">";
static const char objectStartTemplate[] =
        "<Object>"
          "<id>%s</id>";
            //"%%s"       // single printf-escaped %
static const char objectEnd[] = "</Object>";
static const char itemTemplate[] =
        "<InfoItem name=\"%s\">"
          "<value>%s</value>"
        "</InfoItem>";
static const char writeFooter[] =
      "</Objects>"
    "</omi:msg>"
  "</omi:write>"
"</omi:omiEnvelope>";


static const uint16_t MaxItemSize = sizeof(itemTemplate) + MAX_II_NAME_VALUE_SIZE;

// tmp buffer for InfoItem
//static char itemStr[MaxItemSize]; // extra size for name and value

static const uint16_t OMIHttpBufferSize =
    sizeof(writeHeader) + sizeof(writeFooter)
    + (sizeof(objectStartTemplate) + sizeof(objectEnd) + MAX_II_NAME_VALUE_SIZE) * MAX_OBJECTS
    + MaxItemSize * MAX_ITEMS_PER_WRITE;

// tmp buffer for http payload
static char omiHttpBuffer[OMIHttpBufferSize]; 

uint8_t openObjects = 0; // GLOBAL: current number of non-closed <Object>

void omiHeader(){
    DBGSTREAM.printf(FS("[OMI-processing] New O-MI message, Free memory left: %u\r\n"), ESP.getFreeHeap(), OMIHttpBufferSize);

    strcpy(omiHttpBuffer, writeHeader); // replaces the old
    openObjects = 0;
}

bool omiAddObject(const char * objectName) {
    int16_t len = strlen(omiHttpBuffer);
    ++openObjects;
    return snprintf(omiHttpBuffer + len, OMIHttpBufferSize - len, objectStartTemplate, objectName)
        < (OMIHttpBufferSize - len);
}

// Must be used on a buffer that has %s for InfoItem name and value
bool omiAddInfoItem(const char * itemName, const char * valueStr) {
    int16_t len = strlen(omiHttpBuffer);
    return snprintf(omiHttpBuffer + len, OMIHttpBufferSize - len, itemTemplate, itemName, valueStr)
        < (OMIHttpBufferSize - len);
}

bool omiCloseObject(){
    uint16_t bufLen = strlen(omiHttpBuffer);

    if (bufLen + sizeof(objectEnd) < OMIHttpBufferSize) {
        --openObjects;
        strcpy(omiHttpBuffer + bufLen, objectEnd);
        return true;
    }
    return false;
}

bool omiFooter(){
    while (openObjects) if(!omiCloseObject()) return false; // Auto-close open <Object>

    uint16_t bufLen = strlen(omiHttpBuffer);
    if (bufLen + sizeof(writeFooter) < OMIHttpBufferSize) {
        strcpy(omiHttpBuffer + bufLen, writeFooter);
        return true;
    }
    return false;
}

bool createBridgeBootOMI() {
    char valueStr[VALUE_LEN];

    omiHeader();

    omiAddObject("K1");

    omiAddObject(FS("Gateways"));

    // bridges by mac address
    WiFi.macAddress().toCharArray(valueStr, VALUE_LEN);
    omiAddObject(valueStr);

    omiAddInfoItem(FS("HackVersion"), _BuildInfo.src_version);
    omiAddInfoItem(FS("HackBuildTime"), _BuildInfo.time);
    omiAddInfoItem(FS("ESPVersion"), _BuildInfo.env_version);

    String(ESP.getFreeSketchSpace()).toCharArray(valueStr, VALUE_LEN);
    omiAddInfoItem(FS("FreeFlashMemory"), valueStr);

    String(ESP.getFreeHeap()).toCharArray(valueStr, VALUE_LEN);
    omiAddInfoItem(FS("FreeRAM"), valueStr);

    //DBGSTREAM.printf(FS("version: %s\r\n Arduino/ESP version: %s\r\n Build time: %s %s\r\n"),
    //    _BuildInfo.src_version, _BuildInfo.env_version, _BuildInfo.date, _BuildInfo.time);
    //DBGSTREAM.printf(
    //    FS("[SETUP] Free heap RAM: %u  Program flash: Free (for updating): %u Used: %u \r\n")
    //    , ESP.getFreeHeap(), ESP.getFreeSketchSpace(), ESP.getSketchSize());
    return omiFooter();
}

bool createOMI(NodeStr& packetData) {
    char valueStr[VALUE_LEN];

    {
    omiHeader();

    omiAddObject("K1");


    if (packetData.Type == FAIL_OSCILLOSCOPE || packetData.packetLost) {
        omiAddObject(FS("Gateways"));
        WiFi.macAddress().toCharArray(valueStr, VALUE_LEN);
        omiAddObject(valueStr);

        String(packetData.Id).toCharArray(valueStr, VALUE_LEN);
        char* info = FS("CRC error, from: ");
        strcat(info, valueStr);
        omiAddInfoItem("PacketLoss", info);
        return omiFooter();
    }

    DBGSTREAM.printf(FS("[OMI-processing] Creating Object. getNodeName(%i).\r\n"), packetData.Id);
    omiAddObject(getNodeName(packetData.Id));
    }

    // Parse the values TODO
    {switch (packetData.Type) {
        case TH20_OSCILLOSCOPE: {
            DBGSTREAM.printf(FS("[OMI-processing] temp,humi,light InfoItem.\r\n"));

            if (packetData.Last_seq != 0){
                String(packetData.Last_seq).toCharArray(valueStr,VALUE_LEN);
                omiAddInfoItem(FS("THI_PacketNo"), valueStr);
            }

            for (uint8_t idx = 0; idx < 3; ++idx) { // loop temp,humi,illu
                // convert to human readable (*100)
                String((float)packetData.Data.threeInt[idx] * 0.01).toCharArray(valueStr,VALUE_LEN);
                omiAddInfoItem(getTypeName(packetData.Type, idx), valueStr); // TODO: select the data
            }
            break;
        }
        case PIR_OSCILLOSCOPE: // break;
        case CO2S100_OSCILLOSCOPE: // break;
        {
            if (packetData.Last_seq != 0){
                String(packetData.Last_seq).toCharArray(valueStr,VALUE_LEN);
                if (packetData.Type == PIR_OSCILLOSCOPE)
                    omiAddInfoItem(FS("PIR_PacketNo"), valueStr);
                else if (packetData.Type == CO2_OSCILLOSCOPE)
                    omiAddInfoItem(FS("CO2_PacketNo"), valueStr);
            }
        }
        default:
        {
            DBGSTREAM.printf(FS("[OMI-processing] Default InfoItem processing. getTypeName(%i)\r\n"),
                    packetData.Type);

            String(packetData.Data.singleInt).toCharArray(valueStr,VALUE_LEN);
            omiAddInfoItem(getTypeName(packetData.Type), valueStr);
        }
    }}
    // Extra data
    if (packetData.Rssi != 0) {
        String((unsigned int)packetData.Rssi).toCharArray(valueStr,VALUE_LEN);
        omiAddInfoItem(FS("Rssi"), valueStr);
    }

    omiCloseObject();

    return omiFooter(); // should return false if buffer overflowed
}


#ifdef OMI_CERT
bool trySend(HttpsCertificateClient& http) {
#else
bool trySend(HTTPClient& http) {
#endif
    bool isSuccess=false;
    {
    DBGSTREAM.println(F("[HTTP] begin..."));
    
    // configure traged server and url
    // There is support for http basic auth, but it's not very safe without https
#ifdef OMI_CERT
    http.begin(OMI_URL, OMI_CERT_FINGERPRINT, client_crt, client_crt_len, client_key, client_key_len);
#else
    http.begin(OMI_URL, OMI_CERT_FINGERPRINT);
#endif

    //if(!createOMI(packetData))
    //    DBGSTREAM.printf(FS("[OMI-processing] Buffer overflowed, O-MI request larger than %u"),
    //            OMIHttpBufferSize);

    DBGSTREAM.printf(FS("[HTTP] POST... with payload: %s\r\n"), omiHttpBuffer);
    
    http.addHeader(FS("Content-Type"), "text/xml");

    }
    int httpCode = 0;

    // start connection and send HTTP header
    httpCode = http.POST((uint8_t*)omiHttpBuffer, strlen(omiHttpBuffer));


    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        DBGSTREAM.printf(FS("[HTTP] POST... code: %d\r\n"), httpCode);

        // Success
        if(httpCode == HTTP_CODE_OK) isSuccess=true;

        // print result into serial
        String payload = http.getString();
        DBGSTREAM.println(payload);
        
    } else {
        DBGSTREAM.print(F("[HTTP] POST... failed, error: "));
        DBGSTREAM.printf(FS("%s\r\n"), http.errorToString(httpCode).c_str());
    }
    http.end();

    return isSuccess;
}

