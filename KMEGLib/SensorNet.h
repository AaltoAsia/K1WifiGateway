#ifndef SensorNet_H
#define SensorNet_H

#include <Arduino.h>
#include <stdint.h>

#ifndef DBG
#define DBG 0
#endif

char* _getMsgBuf(const __FlashStringHelper* fstr);
#define FS(str) _getMsgBuf(F(str))

enum {
    HDLC_FLAG_BYTE	   = 0x7e,
    HDLC_CTLESC_BYTE	   = 0x7d,
};

// Framer-level dispatch
enum {
    SERIAL_PROTO_ACK = 67,              // 0x43
    SERIAL_PROTO_PACKET_ACK = 68,       // 0x44
    SERIAL_PROTO_PACKET_NOACK = 69,     // 0x45
    SERIAL_PROTO_PACKET_UNKNOWN = 255   // 0xFF
};


typedef struct lqi_beacon_msg {
    uint16_t originaddr;
    int16_t seqno;
    int16_t originseqno;
    uint16_t parent;
    uint16_t cost;
    uint16_t hopcount;
} lqi_beacon_msg_t;

// Other types than in Yggdrasil:
#define FAIL_OSCILLOSCOPE 252

typedef struct _NodeStr NodeStr;

typedef union _Data {
    uint16_t threeInt[3];
    uint16_t singleInt;
} Data_t;

struct _NodeStr{
    Data_t Data;

    uint16_t Id;
    uint16_t Type;
    uint16_t Last_seq;
    uint16_t length;

    int8_t Rssi; // Receive signal strength indicator
    bool packetLost; // latest packet was lost in serial comms
    bool Ack;

}__attribute__((packed));

typedef struct serial_header {
    uint8_t proto;
    uint8_t seqno;
    uint16_t dest;
    uint16_t src;
    uint8_t length;
    uint8_t group;
    uint8_t type;
}__attribute__((packed)) serial_header_t;

// INDICES to the basic packet
#define PROTO 0
#define SEQNO 1
#define DEST1 2
#define DEST0 3
#define SRC1 4
#define SRC0 5
#define LENGTH 6
#define GROUP 7
#define SERTYPE 8

#define SENTYPE1 9
#define SENTYPE0 10
#define SERID5 11
#define SERID4 12
#define SERID3 13
#define SERID2 14
#define SERID1 15
#define SERID0 16
#define NODEID1 17
#define NODEID0 18
#define COUNT1 19
#define COUNT0 20
#define BATT1 21
#define BATT0 22

#define DATA5 23
#define DATA4 24
#define DATA3 25
#define DATA2 26
#define DATA1 27
#define DATA0 28

#define CRC1 29
#define CRC0 30

// Order of temp, humi, light
#define TEMPERATURE_I 0
#define HUMIDITY_I 1
#define LIGHT_I 2

int read_packet(uint8_t *packet);
char getch(void);
void putch(unsigned char buf);
uint8_t crcCheck(uint8_t *data, uint8_t len);
bool getNode(NodeStr * tmp);

#endif
