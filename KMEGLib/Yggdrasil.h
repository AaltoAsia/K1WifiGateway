/*
 */

#ifndef MULTIHOP_OSCILLOSCOPE_H
#define MULTIHOP_OSCILLOSCOPE_H

#include <Arduino.h>

enum {
  /* Sensor Type 98 - 130 */
  AM_OSCILLOSCOPE = 98, 	// 0x62
  BASE_OSCILLOSCOPE = 99, 	// 0x63
	// jh.kang vtt 2013/12
  TH_OSCILLOSCOPE = 100, 	// 0x64
  PIR_OSCILLOSCOPE = 101, 	// 0x65
  CO2_OSCILLOSCOPE = 102, 	//0x66
  VOCS_OSCILLOSCOPE = 103, 	//0x67
  POW_OSCILLOSCOPE = 104, 	//0x68
  US_OSCILLOSCOPE = 105, 	//0x69
  THERMO_LOGGER_OSCILLOSCOPE = 106, //0x6a
  MOBILE_OSCILLOSCOPE = 107, 	//0x6b
  MARKER_OSCILLOSCOPE = 108, 	//0x6c
  SPLUG_OSCILLOSCOPE = 109, 	//0x6d
  EXTENTION_OSCILLOSCOPE = 110, //0x6e
  CSI_OSCILLOSCOPE = 111, 	//0x6f
  TH20_OSCILLOSCOPE = 112, 	// 0x70
  CO2S100_OSCILLOSCOPE = 113, 	//0x71
  SPLUG2_OSCILLOSCOPE = 114, 	//0x72
  SOLAR_OSCILLOSCOPE = 201, 	//0x
  ETYPE_OSCILLOSCOPE = 211, 	//0x
  MAXCO2_OSCILLOSCOPE = 212, 	//0xd4
  PEAK_OSCILLOSCOPE= 213, 	//0xd5
  DUMMY_OSCILLOSCOPE = 250,	// 0xfa
  INFO_OSCILLOSCOPE = 251	// 0xfb
};

enum {
  /* RF Interval */
  //DEFAULT_INTERVAL = 61440U,
  DEFAULT_INTERVAL = 5120U,
  FX_INTERVAL = 10240U,
  BASE_INTERVAL = 20240U,
  TH_INTERVAL = 1024U,
  POW_INTERVAL = 10240U,
  //CO2_INTERVAL = 61440U,
  CO2_INTERVAL = 1024U,
  VOCS_INTERVAL = 1024U,
  //PIR_INTERVAL = 20480U,
  PIR_INTERVAL = 20480U,
  US_INTERVAL = 1024U,
  DOOR_INTERVAL = 3072U,
  THERMO_LOGGER_INTERVAL = 10240U,
  SPLUG_INTERVAL = 10240U,
  EXTENTION_INTERVAL = 10240U,
  MOBILE_INTERVAL = 512U,
  MARKER_INTERVAL = 512U,

  // KETI
  ETYPE_INTERVAL = 5120U,
  DUMMY_INTERVAL = 1024U,
  INFO_INTERVAL = 1024U
};


enum {
  SENSOR_READINGS = 1,
  MARKER_READINGS = 4
};

enum {
  /* Public */
  TTL = 10,
  //
  /* Keti Solar Project */
  COMMAND_SIZE = 8,
  SETUP_TIME = 5,
  DATA_SIZE = 70
    /* Keti Solar Project */
};

typedef struct base_oscilloscope {
  uint16_t type;       // Sensor type, constants above
  uint8_t serialId[6]; //
  uint16_t id;       // Node ID
  uint16_t count;   // Number of packet in sequence
  uint16_t battery; // Battery voltage
} base_info_t;

typedef struct oscilloscope {
  base_info_t info;
  uint16_t channel;
  uint16_t gId;
} oscilloscope_t;

typedef struct th_oscilloscope {
  base_info_t info;
  uint16_t temp[SENSOR_READINGS];
  uint16_t humi[SENSOR_READINGS];
  uint16_t illu[SENSOR_READINGS];
} th_oscilloscope_t;

typedef struct pir_oscilloscope {
  base_info_t info;
  uint16_t interrupt;
  uint16_t dummy1;
  uint16_t dummy2;
} pir_oscilloscope_t;

typedef struct co2_oscilloscope {
  base_info_t info;
  uint16_t readings[SENSOR_READINGS];
  uint16_t dummy1;
  uint16_t dummy2;
} co2_oscilloscope_t;

typedef struct maxco2_oscilloscope {
  base_info_t info;
  uint16_t temp;
  uint16_t humi;
  uint16_t co2;
} maxco2_oscilloscope_t;

typedef struct vocs_oscilloscope {
  base_info_t info;
  uint16_t readings[SENSOR_READINGS];
  uint16_t dummy1;
  uint16_t dummy2;
} vocs_oscilloscope_t;

typedef struct marker {
  uint16_t id;
  uint8_t rssi;
  uint8_t seq;
} marker_t;

typedef struct marker_oscilloscope {
  base_info_t info;
  marker_t infra[MARKER_READINGS];
} marker_oscilloscope_t;

typedef struct mango_mobile_oscillsocope {
  uint8_t serialI[6];
  uint16_t mobileId;
  uint16_t seqno;
} mango_mobile_oscilloscope_t;

typedef struct pow_oscilloscope {
  base_info_t info;
  uint16_t readings[SENSOR_READINGS];
  uint16_t accumulate_watt;
  uint16_t port_state;
} pow_oscilloscope_t;

typedef struct us_oscilloscope {
  base_info_t info;
  uint16_t readings[SENSOR_READINGS];
  uint16_t dummy1;
  uint16_t dummy2;
} us_oscilloscope_t;

typedef struct thermo_logger_oscilloscope {
  base_info_t info;
  uint16_t baseAdc; 
  uint16_t Adc1; 
  uint16_t Adc2; 
  uint16_t Adc3; 
  uint16_t Adc4; 
  uint16_t Adc5; 
  uint16_t Adc6; 
  uint16_t Adc7; 
} thermo_logger_oscilloscope_t;

typedef struct splug2_oscilloscope {
  base_info_t info;
  uint32_t watt;
  uint32_t accumulate;
  uint32_t overcount;
} splug2_oscilloscope_t;


typedef struct splug_oscilloscope {
  base_info_t info;
  uint8_t current[3];
  uint8_t raEnergy[3];
  uint8_t rvaEnergy[3];
  uint8_t peak[3];
} splug_oscilloscope_t;

typedef struct etype_meter {
  base_info_t info;
  uint32_t validCurrent;
  uint32_t invalidCurrent;
  uint32_t validTotalCurrent;
  uint32_t invalidTotalCurrent;
} etype_oscilloscope_t;

typedef struct dummy_oscilloscope {
  base_info_t info;
  uint8_t data[4];
} dummy_oscilloscope_t;

typedef struct info_oscilloscope {
  base_info_t info;
  uint8_t    options;
  uint16_t        parent;
  uint16_t         etx;
} info_oscilloscope_t;

typedef struct cmd {
  uint16_t opcode;
  uint16_t opreand_h;
  uint16_t opreand_l;
  uint16_t dest;
  uint8_t seq;
  uint8_t ttl;
} cmd_t;

typedef struct reply_msg {
  //uint16_t counter;
  uint16_t cmd_type;
  uint16_t count;
  uint16_t src;
  uint16_t dest;
  uint16_t arg;
} reply_msg_t;

typedef struct csi_oscilloscope {
  base_info_t info;
  uint8_t actAvr[3];
  uint8_t stdAvr[3];
  uint8_t status[3];
  //struct status{
  //};
} csi_oscilloscope_t;

typedef struct peak_meter {
  base_info_t info;
  uint32_t predictedPower;
  uint32_t currentPower;
  uint32_t demandTime;
  uint32_t reserved;
} peak_oscilloscope_t;

#endif
