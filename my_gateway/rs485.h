/*
 * rs485.h
 *
 *  Created on: 18 lut 2026
 *      Author: kleczynskiw
 */
// rs485.h
#ifndef RS485_H
#define RS485_H
#include "crc.h"
#include <stdint.h>

#define MAX_PAYLOAD 32
#define BUF_SIZE 64
#define START_BYTE 0x7E

typedef struct {
    uint8_t sender;
    uint8_t destination;
    uint8_t child_id;
    uint8_t command;
    uint8_t ack;
    uint8_t type;
    uint8_t length;
    uint8_t payload[MAX_PAYLOAD];
} mys_msg_t;

void rs485_init();
void rs485_tx(uint8_t link);
void rs485_rx(uint8_t link);
void rs485_send_msg(uint8_t link, mys_msg_t *msg);
uint8_t rs485_receive_byte(uint8_t link, uint8_t b, mys_msg_t *msg);
void uart_send(volatile uint8_t *udr, uint8_t data, volatile uint8_t *ucsra);

#endif

