
#include "Yggdrasil.h"

#include <Arduino.h>
#include <pgmspace.h>
#include "SensorNet.h"
#include "K1Building.h"

#ifndef DBGSTREAM
#define DBGSTREAM DummySerial
class DummySerial_t {
    // struct: int (*printf)(const char * format, ...);
    public:
    int printf(const char * format, ...){return 0;}
};


DummySerial_t DummySerial;
#endif

#ifndef YGGDRASIL
#warning Connect YGGDRASIL UART to some serial and put it in the Makefile or this source
#define YGGDRASIL Serial1
#endif


// DBGSTREAM output
//#define DBG 1

#define MAX_PACKET_SIZE 60

char _msgBuf[120];
char* _getMsgBuf(const __FlashStringHelper* fstr) {
    strncpy_P(_msgBuf, reinterpret_cast<PGM_P>(fstr), sizeof(_msgBuf));
    _msgBuf[sizeof(_msgBuf) - 1] = '\0';
    return _msgBuf;
}


// This function converts 24bit data to 32bit data
// @param byte_3 : big endian 24bit array pointer
int32_t _24to_32(uint8_t *byte_3){

    int32_t data = 0;

    // MSB recognition
    if(*byte_3 & 0x80){
        data = 0xff;
        data <<= 8;
    }

    data |= *byte_3;
    data <<= 8;
    data |= *(byte_3+1);
    data <<= 8;
    data |= *(byte_3+2);

    return data;
}



static uint8_t post_char=0;  // last loop run input byte

int read_packet(uint8_t *packet)
{
    uint8_t size=0;
    byte started_flag=0;  // packet start found 
    uint8_t data;         // input byte
    byte exit_counter=10; // exit after this goes to zero without a packet start

    while(exit_counter > 0)
    {
        data=getch();
        DBGSTREAM.printf("%02X ", data);

        if(started_flag == 1){

            // END FRAME
            if(data == HDLC_FLAG_BYTE){
                DBGSTREAM.println(" <E>");
                started_flag=0;
                return size;
            }

            packet[size++]=data; // size points next empty slot

            // Remove escape character combinations
            if(data == 0x5e && post_char == HDLC_CTLESC_BYTE){
                // 7d  5e [ ]
                size--;
                // 7d [5e]
                packet[size-1]=0x7e;
                // 7e [5e]
                DBGSTREAM.print(F("-- [7E] "));
            }
            else if(data == 0x5d && post_char == HDLC_CTLESC_BYTE){
                size--;
                packet[size-1]=0x7d;
                DBGSTREAM.print(F("-- [7D] "));
            }

            // Frame end missing or larger than expected, beginning might be valid
            if (size >= MAX_PACKET_SIZE) break;

        } else if (data != HDLC_FLAG_BYTE) {
            --exit_counter;
        }


        // START FRAME
        if(post_char == HDLC_FLAG_BYTE){
            switch(data){
                // JOINED END-START FRAME, then last call ate the marker.
                // also fixes desync state where last message was parsed as empty (... 7e 7e ...)
                // also post_char is global
                case HDLC_FLAG_BYTE:
                    break;
                case SERIAL_PROTO_ACK:
                case SERIAL_PROTO_PACKET_ACK:
                case SERIAL_PROTO_PACKET_NOACK:
                case SERIAL_PROTO_PACKET_UNKNOWN:
                default :            // FIXME: Is it good to ignore invalid
                    packet[0]=data;
                    started_flag=1;
                    size = 1;
                    ++exit_counter; // Otherwise counter might be left zero
                    DBGSTREAM.print(" <S> ");
                    break;
            }
        }

        post_char=data;

    }

    return size;
}

static uint16_t crcByte(uint16_t crc, uint8_t b){
    crc = (uint8_t )(crc >> 8) | (crc << 8);
    crc ^= b;
    crc ^= (uint8_t )(crc & 0xff) >> 4;
    crc ^= crc << 12;
    crc ^= (crc & 0xff) << 5;
    return crc;
}

uint8_t crcCheck(uint8_t *data, uint8_t len){
    int i;
    uint16_t crc = 0;

    for(i = 0 ; i < len - 2; i++)
        crc = crcByte(crc, *(data+i));

    if(crc ==((data[len-1] <<8 ) | (data[len-2])))
        return 1;

#if DBG
    DBGSTREAM.printf(FS("Calcuated crc : %x \r\n"), crc);
    DBGSTREAM.printf(FS("Received crc  : %x \r\n"), (data[len-1]<<8) | (data[len-2]) );
#endif

    return 0;
}

NodeStr * getArrayPos(NodeStr * accData, uint8_t len, uint16_t id)
{
    for(uint8_t i = 0; i <= len; i++)
    {
        if(i == len){
            break;//return &accData[len];
        }
        if(accData[i].Id == id){
            return &accData[i];
        }
    }
    
    //initializd necessary values
    accData[len].Id = id;
    //accData[len].Type = 0;
    accData[len].treeCount = 0;
    accData[len].co2Count = 0;
    accData[len].pirCount = 0; //so that we don't access null values
    return &accData[len];
}

uint8_t packet[MAX_PACKET_SIZE];
bool getNode(NodeStr * accData, uint8_t len)
{
    uint16_t id, seq;
    int size;

    bool sch = false;

    serial_header_t * info;
    size = read_packet(packet);


    if(size<=0) {
        DBGSTREAM.println(F("[Ygg] No data"));
        return false;
    }

    switch(packet[0]){
        case SERIAL_PROTO_ACK:
            break;

        case SERIAL_PROTO_PACKET_ACK:

#if DBG
            DBGSTREAM.println(F("ACK Requsted packet"));
#endif
            putch(HDLC_FLAG_BYTE);
            putch(SERIAL_PROTO_ACK);

            break;

        case SERIAL_PROTO_PACKET_NOACK:
            break;

        case SERIAL_PROTO_PACKET_UNKNOWN:
            break;

        default:
            sch = true;
            break;

    }

    if(sch){
        DBGSTREAM.println(F("[Ygg] SERIAL PACKET ERROR!\r\n"));
        return false;

    } else {
        DBGSTREAM.printf(FS("[Ygg] Success, packet size: %u \r\n"), size);
    }
    info = (serial_header_t*)packet;


    // Oscilloscope Message type
    id = packet[NODEID1] << 8;
    id |= packet[NODEID0];
    seq = packet[COUNT1] << 8;
    seq |= packet[COUNT0];

    uint16_t type = packet[SENTYPE1] << 8;
    type |= packet[SENTYPE0];

    // packet filter :
    // Base Oscilloscope = 0x63
    if(type == BASE_OSCILLOSCOPE){
        DBGSTREAM.println(F("BASE"));
        return false;
    }

    NodeStr * tmp;

    if(crcCheck(packet, size)){
        tmp = getArrayPos(accData, len, id);
        tmp->Id = id;
        tmp->Type = type;
        tmp->Last_seq = seq;
        memset(tmp->intValues, 0, 5);
        tmp->packetLost = false;

        if(packet[PROTO] == SERIAL_PROTO_PACKET_ACK )
            tmp->Ack = true;
        else
            tmp->Ack = false;

        if( packet[SERTYPE] % 2)
            tmp->Rssi = packet[size-3];
        else
            tmp->Rssi = 0;

#if DBG
        DBGSTREAM.printf(FS("Received Node : %d\r\n"), tmp->Id);
        DBGSTREAM.printf(FS("Last_seq     : %x\r\n"), tmp->Last_seq);
#endif

    }
    else{
        // CHECK FCS error, SERIAL FAULT

        DBGSTREAM.println(F("[Ygg] CRC FAILURE IN SERIAL!\r\n"));

        if (getNodeIndex(id) != 0xFFFF) {
            tmp = getArrayPos(accData, len, id);
            tmp->Id = id;
            tmp->Type = FAIL_OSCILLOSCOPE;
            tmp->Last_seq = 0;
            tmp->Rssi = 0; // dBm - offset?
            memset(tmp->intValues, 0, 5);
            tmp->packetLost = true;

            return true; // we have a failure packet to send
        } else {
            return false;
        }
    }

    base_info_t * baseInfo = (base_info_t *)&packet[sizeof(serial_header_t)];

    baseInfo->type = packet[sizeof(serial_header_t)+1];
    baseInfo->type = packet[sizeof(serial_header_t)];

    tmp->Type = baseInfo->type;

#if DBG
    // packet information 
    DBGSTREAM.printf(FS("packet type  : %x\r\n"), info->type );
    DBGSTREAM.printf(FS("Oscill type  : %x\r\n"), baseInfo->type );
    DBGSTREAM.printf(FS("data   length: %x\r\n"), info->length );
    DBGSTREAM.printf(FS("packet length: %x\r\n"), info->length + sizeof(serial_header_t) );
#endif	

    switch (tmp->Type) {
        case PIR_OSCILLOSCOPE: {
            tmp->pirCount++;
            // casting to struct has fundamental problems
            //pir_oscilloscope_t* pir = (pir_oscilloscope_t*)&packet[sizeof(serial_header_t)];
            //tmp->Data.singleInt = pir->interrupt;
            tmp->intValues[4] = packet[DATA5] << 8 | packet[DATA4];
            break;
        }
        case TH20_OSCILLOSCOPE: {
            tmp->treeCount++;
            // casting to struct has fundamental problems
            //th_oscilloscope_t* th = (th_oscilloscope_t*)&packet[sizeof(serial_header_t)];
            ////tmp->Data.threeInt = {th->temp[0], th->humi[0], th->illu[0]};

            //memcpy(tmp->Data.threeInt, packet+DATA5, 3 * sizeof(uint16_t)); // three shorts

            uint16_t tempRead = packet[DATA5] << 8 | packet[DATA4];
            tmp->intValues[2] += (packet[DATA1] << 8 | packet[DATA0]) * 100;

            // Conversions according to datasheets of Sht2x
            uint32_t temp = 17572;
            temp *= (uint32_t)(tempRead & 0xFFFC);
            temp >>= 16;

            tmp->intValues[1] += (uint16_t) temp;

            uint16_t humidRead = packet[DATA3] << 8 | packet[DATA2];

            // Conversions according to datasheets of Sht2x
            uint32_t humi = (uint32_t)12500;
            humi *= (humidRead & 0xFFFC);
            humi >>= 16;

            tmp->intValues[0] += (uint16_t) humi * 10; // Don't know why the decimal place is wrong

            // TODO: ADC conversion of light sensor?
            break;
        }
        case CO2S100_OSCILLOSCOPE: {
            // casting to struct has fundamental problems
            //co2_oscilloscope_t* co2 = (co2_oscilloscope_t*)&packet[sizeof(serial_header_t)];
            //tmp->Data.singleInt = co2->readings[0];
            tmp->co2Count++;
            tmp->intValues[3] += packet[DATA5] << 8 | packet[DATA4];
            break;
        }
        default:
            DBGSTREAM.println(F("[SensorNet] Unknown oscilloscope type, cannot parse data!"));
    }

    return true;
}



