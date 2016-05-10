/*
 * K4Building constants and metadata header
 */
#ifndef K4BUILDING_H
#define K4BUILDING_H

#include <Yggdrasil.h>
#include <Arduino.h>




uint16_t getNodeIndex(uint16_t id) ;

char* getNodeName(uint16_t id) ;

const char* getTypeName(uint8_t typeId, uint8_t extraId=0) ;

#endif // header
