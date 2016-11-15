
#include "K1Building.h"
#include <pgmspace.h>
#include "SensorNet.h" // only for FS() macro TODO: clean

const char unknown[] PROGMEM = "unknown";
const char empty[]   PROGMEM = "";

// 4TH FLOOR
const char r4401[] PROGMEM = "414b";
const char r4402[] PROGMEM = "414c";
const char r4403[] PROGMEM = "414a";
const char r4404[] PROGMEM = "402_Corridor";
const char r4405[] PROGMEM = "413";
const char r4406[] PROGMEM = "406";
//const char r4407[] PROGMEM = "";
const char r4408[] PROGMEM = "405";
const char r4409[] PROGMEM = "404";
const char r4410[] PROGMEM = "415b"; // Otaniemi3d error? 
const char r4411[] PROGMEM = "403";
const char r4412[] PROGMEM = "402a"; // Otaniemi3d: 402
const char r4413[] PROGMEM = "415a"; // Otaniemi3d error?
const char r4414[] PROGMEM = "405_Corridor";
const char r4415[] PROGMEM = "414_Corridor";
//const char r4416[] PROGMEM = "";
const char r4417[] PROGMEM = "408"; // Otaniemi3d: 408a
const char r4418[] PROGMEM = "412a";

// 3TH FLOOR
const char r4301[] PROGMEM = "301d";
const char r4302[] PROGMEM = "301c";
const char r4303[] PROGMEM = "302b";
const char r4304[] PROGMEM = "303a";
const char r4305[] PROGMEM = "303b";
const char r4306[] PROGMEM = "304a";
const char r4307[] PROGMEM = "301_Corridor";
const char r4308[] PROGMEM = "311_Kitchen";
const char r4309[] PROGMEM = "309";
const char r4310[] PROGMEM = "310";
const char r4311[] PROGMEM = "311";
const char r4312[] PROGMEM = "323";
const char r4313[] PROGMEM = "324";
const char r4314[] PROGMEM = "325b";
const char r4315[] PROGMEM = "326";
const char r4316[] PROGMEM = "323_Corridor";
const char r4317[] PROGMEM = "325a_Corridor";
const char r4318[] PROGMEM = "326_Corridor";
const char r4319[] PROGMEM = "312";
const char r4320[] PROGMEM = "313";
const char r4321[] PROGMEM = "314";
const char r4322[] PROGMEM = "315";
const char r4323[] PROGMEM = "316";
const char r4324[] PROGMEM = "318";
const char r4325[] PROGMEM = "344_Corridor";
const char r4326[] PROGMEM = "345_Corridor";
const char r4327[] PROGMEM = "338_Corridor";
const char r4328[] PROGMEM = "331";
const char r4329[] PROGMEM = "332";
const char r4330[] PROGMEM = "333";
const char r4331[] PROGMEM = "335";
const char r4332[] PROGMEM = "336";
//const char r4333[] PROGMEM = "";
//const char r4334[] PROGMEM = "";
const char r4335[] PROGMEM = "337";
//const char r4336[] PROGMEM = "";
const char r4337[] PROGMEM = "336";
const char r4338[] PROGMEM = "341c";

// 2ND FLOOR
const char r4201[] PROGMEM = "201";
const char r4202[] PROGMEM = "202";
const char r4203[] PROGMEM = "203c";
const char r4204[] PROGMEM = "203b";
const char r4205[] PROGMEM = "204a";
const char r4206[] PROGMEM = "204d";
//const char r4207[] PROGMEM = "238a"; //testing
const char r4208[] PROGMEM = "201_Corridor";
const char r4209[] PROGMEM = "214_Corridor_Stairs";
const char r4210[] PROGMEM = "209";
const char r4211[] PROGMEM = "210";
const char r4212[] PROGMEM = "211b";
const char r4213[] PROGMEM = "211a";
const char r4214[] PROGMEM = "213_Bottom";
const char r4215[] PROGMEM = "215";
const char r4216[] PROGMEM = "213_Top";
const char r4217[] PROGMEM = "233a_Corridor";
const char r4218[] PROGMEM = "214_CorridorMiddle";
const char r4219[] PROGMEM = "216";
const char r4220[] PROGMEM = "213a_SunDeck";
//const char r4221[] PROGMEM = "";
//const char r4222[] PROGMEM = "";
const char r4223[] PROGMEM = "237c";
const char r4224[] PROGMEM = "216_Corridor";
//const char r4225[] PROGMEM = "";
const char r4226[] PROGMEM = "222";
const char r4227[] PROGMEM = "223";
const char r4228[] PROGMEM = "224";
const char r4229[] PROGMEM = "225";
const char r4230[] PROGMEM = "226";
const char r4231[] PROGMEM = "227";
const char r4232[] PROGMEM = "228";
const char r4233[] PROGMEM = "236a_Corridor";
//const char r4234[] PROGMEM = "229_Corridor";
const char r4235[] PROGMEM = "233";
const char r4236[] PROGMEM = "221b_Corridor";
const char r4237[] PROGMEM = "239b";
const char r4238[] PROGMEM = "238b";
const char r4239[] PROGMEM = "239a";

// 1ST FLOOR
const char r4601[] PROGMEM = "148";
const char r4602[] PROGMEM = "149";
const char r4603[] PROGMEM = "150";
const char r4604[] PROGMEM = "151";
const char r4605[] PROGMEM = "152";
const char r4606[] PROGMEM = "153";
const char r4607[] PROGMEM = "154a";
const char r4608[] PROGMEM = "154b";
const char r4609[] PROGMEM = "155";
const char r4610[] PROGMEM = "156";
const char r4611[] PROGMEM = "157";
//const char r4612[] PROGMEM = "";
//const char r4613[] PROGMEM = "";
const char r4614[] PROGMEM = "144";
const char r4615[] PROGMEM = "143";
const char r4616[] PROGMEM = "142";
const char r4617[] PROGMEM = "141";
const char r4618[] PROGMEM = "140";
const char r4619[] PROGMEM = "139";
const char r4620[] PROGMEM = "138";
const char r4621[] PROGMEM = "137";
const char r4622[] PROGMEM = "114";
const char r4623[] PROGMEM = "115";
const char r4624[] PROGMEM = "116";
const char r4625[] PROGMEM = "105";
const char r4626[] PROGMEM = "104b";
const char r4627[] PROGMEM = "104a";
const char r4628[] PROGMEM = "103";
const char r4629[] PROGMEM = "102b";
//const char r4630[] PROGMEM = "";
const char r4631[] PROGMEM = "160a_GuildRoom";
const char r4632[] PROGMEM = "124";
const char r4633[] PROGMEM = "144a_Corridor";
const char r4634[] PROGMEM = "154b_Corridor";
const char r4635[] PROGMEM = "149_Corridor";
const char r4636[] PROGMEM = "148_Corridor";
const char r4637[] PROGMEM = "160a_Corridor";
const char r4638[] PROGMEM = "109b";
const char r4639[] PROGMEM = "Cafeteria_Kitchen";
const char r4640[] PROGMEM = "Lobby";
const char r4641[] PROGMEM = "111_Corridor";
const char r4642[] PROGMEM = "116_Corridor";
const char r4643[] PROGMEM = "H165a_Corridor";


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
    unknown,
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

    // 3TH FLOOR
    // 43xx
    r4301,
    r4302,
    r4303,
    r4304,
    r4305,
    r4306,
    r4307,
    r4308,
    r4309,
    r4310,
    r4311,
    r4312,
    r4313,
    r4314,
    r4315,
    r4316,
    r4317,
    r4318,
    r4319,
    r4320,
    r4321,
    r4322,
    r4323,
    r4324,
    r4325,
    r4326,
    r4327,
    r4328,
    r4329,
    r4330,
    r4331,
    r4332,
    empty,
    empty,
    r4335,
    empty,
    r4337,
    r4338,

    // 2ND FLOOR
    // 42xx
    r4201,
    r4202,
    r4203,
    r4204,
    r4205,
    r4206,
    r4207,
    r4208,
    r4209,
    r4210,
    r4211,
    r4212,
    r4213,
    r4214,
    r4215,
    r4216,
    r4217,
    r4218,
    r4219,
    r4220,
    empty,
    empty,
    r4223,
    r4224,
    empty,
    r4226,
    r4227,
    r4228,
    r4229,
    r4230,
    r4231,
    r4232,
    r4233,
    r4234,
    r4235,
    r4236,
    r4237,
    r4238,
    r4239,

    // 1ST FLOOR
    // 46xx
    r4601,
    r4602,
    r4603,
    r4604,
    r4605,
    r4606,
    r4607,
    r4608,
    r4609,
    r4610,
    r4611,
    empty,
    empty,
    r4614,
    r4615,
    r4616,
    r4617,
    r4618,
    r4619,
    r4620,
    r4621,
    r4622,
    r4623,
    r4624,
    r4625,
    r4626,
    r4627,
    r4628,
    r4629,
    empty,
    r4631,
    r4632,
    r4633,
    r4634,
    r4635,
    r4636,
    r4637,
    r4638,
    r4639,
    r4640,
    r4641,
    r4642,
    r4643,

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
    int16_t range_start = 1;

    // 4TH FLOOR
    RANGE(4401, 4418)
    RANGE(4531, 4533)

    // 3TH FLOOR
    RANGE(4301, 4338)

    // 2ND FLOOR
    RANGE(4201, 4239)

    // 1ST FLOOR
    RANGE(4601, 4643)

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
