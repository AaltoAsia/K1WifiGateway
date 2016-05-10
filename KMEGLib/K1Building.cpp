
#include "K1Building.h"
#include <pgmspace.h>
#include "SensorNet.h" // only for FS() macro TODO: clean

const char unknown[] PROGMEM = "unknown";
const char empty[]   PROGMEM = "";

// 4TH FLOOR
const char r4401[] PROGMEM = "414b";
const char r4402[] PROGMEM = "414c";
const char r4403[] PROGMEM = "414a";
const char r4404[] PROGMEM = "4thFloorCorridor1";
const char r4405[] PROGMEM = "413";
const char r4406[] PROGMEM = "406";

const char r4408[] PROGMEM = "405";
const char r4409[] PROGMEM = "404";
const char r4410[] PROGMEM = "415b"; // Otaniemi3d error? 
const char r4411[] PROGMEM = "403";
const char r4412[] PROGMEM = "402a"; // Otaniemi3d: 402
const char r4413[] PROGMEM = "415a"; // Otaniemi3d error?
const char r4414[] PROGMEM = "4thFloorCorridor2";
const char r4415[] PROGMEM = "4thFloorCorridor3";

const char r4417[] PROGMEM = "408"; // Otaniemi3d: 408a
const char r4418[] PROGMEM = "412a";

// TESTING
const char r4207[] PROGMEM = "TestBlue";
const char r4234[] PROGMEM = "TestYellow";
const char r4518[] PROGMEM = "TestBridge";

// TYPES
const char temp[] = "temperature";
const char humi[] = "humidity";
const char illu[] = "light";
const char pir[] = "pir";
const char co2[] = "co2";

const char* NodeNames[] = {
    // 4TH FLOOR
    // 44xx
    r4401,
    r4402,
    r4403,
    r4404,
    r4405,
    r4406,
    empty,
    r4408,
    r4409,
    r4410,
    r4411,
    r4412,
    r4413,
    r4414,
    r4415,
    empty,
    r4417,
    r4418,
    // 453x // These are wiz bridges on the 4th floor
    r4404,  // 18,  shares same names in the o-df but bridges are under different Object
    r4414, // 19
    r4415, // 20

    // TEMPORARY TEST
    r4207,
    r4234,
    r4518,

    // TODO: Others
};
const uint16_t numberOfNodes = sizeof(NodeNames);
//const char** Nodes44xx = NodeNames;
//const char** Nodes453x = NodeNames +19;

// [start,end]: start inclusive, end inclusive
#define RANGE(start, end) \
    if (start <= id && id <= end)\
        return (id - start + range_start);\
    range_start += end - start + 1;

// Get array compatible index from node id
uint16_t getNodeIndex(uint16_t id) {
    int16_t range_start = 0;

    // 4TH FLOOR
    RANGE(4401, 4418)
    RANGE(4531, 4533)

    // For testing
    RANGE(4207, 4207)
    RANGE(4234, 4234)
    RANGE(4518, 4518)
    return 0xFFFF; // unknown
}


// FIXME crash getTypeName(99)

//#define GET_FLASH_ARR(arr, ind) strcpy_P(_msgBuf, (char*)pgm_read_word(&(arr[ind])));
char* getNodeName(uint16_t id) {
    int16_t ret = getNodeIndex(id);
    if (ret < numberOfNodes) return _getMsgBuf(FPSTR(NodeNames[ret]));
    else return _getMsgBuf(FPSTR(unknown));
}

const char* getTypeName(uint8_t typeId, uint8_t extraId) {
    switch (typeId) {
        case TH20_OSCILLOSCOPE:
            switch (extraId) {
                case 0: return temp;
                case 1: return humi;
                case 2: return illu;
            }
        case PIR_OSCILLOSCOPE:
            return pir;
        case CO2S100_OSCILLOSCOPE:
            return co2;
        default:
            return unknown;
    }

}

//char* getTypeName(uint8_t typeId, uint8_t extraId) {
//    switch (typeId) {
//        case TH20_OSCILLOSCOPE:
//            switch (extraId) {
//                case 0: return _getMsgBuf(FPSTR(temp));
//                case 1: return _getMsgBuf(FPSTR(humi));
//                case 2: return _getMsgBuf(FPSTR(illu));
//            }
//        case PIR_OSCILLOSCOPE:
//            return _getMsgBuf(FPSTR(pir));
//        case CO2S100_OSCILLOSCOPE:
//            return _getMsgBuf(FPSTR(co2));
//        default:
//            return _getMsgBuf(FPSTR(unknown));
//    }
//
//}
