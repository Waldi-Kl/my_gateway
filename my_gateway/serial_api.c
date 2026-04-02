/*
 * serial_api.c
 *
 *  Created on: 18 lut 2026
 *      Author: kleczynskiw
 */


// serial_api.c
#include "serial_api.h"
#include "rs485.h"
#include <avr/io.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>


void send_to_ha(mys_msg_t *msg){
    char buf[96]; char payload[32]={0};

    if(msg->length==1) snprintf(payload,sizeof(payload),"%d",msg->payload[0]);
    else if(msg->length==2){
        uint16_t v = (msg->payload[0]<<8)|msg->payload[1];
        snprintf(payload,sizeof(payload),"%u",v);
    }
    else if(msg->length>2){
        memcpy(payload,msg->payload,msg->length);
        payload[msg->length]=0;
    }

    snprintf(buf,sizeof(buf),"%d;%d;%d;%d;%d;%s\r\n",
             msg->sender,
             msg->child_id,
             msg->command,
             msg->ack,
             msg->type,
             payload);

    for(uint8_t i=0;i<strlen(buf);i++)
        uart_send(&UDR0,buf[i],&UCSR0A);
}

uint8_t parse_mys_line(char *line, mys_msg_t *msg){
    uint8_t field = 0;
    char *start = line;

    // domyślne wartości
    msg->sender = 0;  // gateway = 0
    msg->destination = 0;
    msg->child_id = 0;
    msg->command = 0;
    msg->ack = 0;
    msg->type = 0;
    msg->length = 0;
    memset(msg->payload, 0, MAX_PAYLOAD);

    for(char *p = line; *p; p++){
        if(*p == ';'){
            *p = 0;
            switch(field){
                case 0: msg->destination = atoi(start); break;
                case 1: msg->child_id   = atoi(start); break;
                case 2: msg->command    = atoi(start); break;
                case 3: msg->ack        = atoi(start); break;
                case 4: msg->type       = atoi(start); break;
            }
            field++;
            start = p + 1;
        }
    }

    // reszta po ostatnim średniku → payload
    strncpy((char*)msg->payload, start, MAX_PAYLOAD);
    msg->length = strlen((char*)msg->payload);

    return (field >= 5);
}

