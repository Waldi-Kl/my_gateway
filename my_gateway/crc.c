/*
 * crc.c
 *
 *  Created on: 18 lut 2026
 *      Author: kleczynskiw
 */

// crc.c
#include "crc.h"

uint8_t crc8(uint8_t *data, uint8_t len){
    uint8_t crc=0;
    for(uint8_t i=0;i<len;i++) crc ^= data[i];
    return crc;
}

