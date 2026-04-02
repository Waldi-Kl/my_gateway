/*
 * serial_api.h
 *
 *  Created on: 18 lut 2026
 *      Author: kleczynskiw
 */
// serial_api.h
#ifndef SERIAL_API_H
#define SERIAL_API_H
#include "rs485.h"
void send_to_ha(mys_msg_t *msg);
uint8_t parse_mys_line(char *line, mys_msg_t *msg);
#endif

