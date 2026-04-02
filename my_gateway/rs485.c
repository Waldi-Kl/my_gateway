/*
 * rs485.c
 *
 *  Created on: 18 lut 2026
 *      Author: kleczynskiw
 */


// rs485.c
#include "rs485.h"
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

// DE/RE piny dla trzech linków
#define DE1 1 // 36 - UART1
#define DE2 3 // 34 - UART2
#define DE3 5 // 32 - UART3

static uint8_t buf1[BUF_SIZE]; static uint8_t idx1=0;
static uint8_t buf2[BUF_SIZE]; static uint8_t idx2=0;
static uint8_t buf3[BUF_SIZE]; static uint8_t idx3=0;

// --- DE/RE ---
void rs485_tx(uint8_t link){
    switch(link){
        case 1: PORTC |= (1<<DE1); break;
        case 2: PORTC |= (1<<DE2); break;
        case 3: PORTC |= (1<<DE3); break;
    }
    _delay_us(10);
}

void rs485_rx(uint8_t link){
    switch(link){
        case 1: PORTC &= ~(1<<DE1); break;
        case 2: PORTC &= ~(1<<DE2); break;
        case 3: PORTC &= ~(1<<DE3); break;
    }
    _delay_us(10);
}

void rs485_init(){
    DDRC |= (1<<DE1)|(1<<DE2)|(1<<DE3);
    rs485_rx(1); rs485_rx(2); rs485_rx(3);
}

// --- UART send byte (link1/2/3 u¿ywa UDR1/2/3) ---
void uart_send(volatile uint8_t *udr, uint8_t data, volatile uint8_t *ucsra){
    while(!(*ucsra & (1<<UDRE0)));
    *udr = data;
}

void rs485_send_msg(uint8_t link, mys_msg_t *msg){
    volatile uint8_t *udr=NULL;
    volatile uint8_t *ucsra=NULL;
    uint8_t txc_bit = 0; // Pomocnicza zmienna na bit TXC

    switch(link){
        case 1: udr=&UDR1; ucsra=&UCSR1A; txc_bit=TXC1; break;
        case 2: udr=&UDR2; ucsra=&UCSR2A; txc_bit=TXC2; break;
        case 3: udr=&UDR3; ucsra=&UCSR3A; txc_bit=TXC3; break;
    }

    // 1. Prze³¹cz pin DE/RE na nadawanie
    rs485_tx(link);

    // 2. Skasuj flagê TXC przed wysy³k¹ (wpisuj¹c logiczn¹ 1)
    *ucsra |= (1 << txc_bit);

    // 3. Wyœlij wszystkie bajty ramki
    uart_send(udr, START_BYTE, ucsra);
    uart_send(udr, msg->sender, ucsra);
    uart_send(udr, msg->destination, ucsra);
    uart_send(udr, msg->child_id, ucsra);
    uart_send(udr, msg->command, ucsra);
    uart_send(udr, msg->ack, ucsra);
    uart_send(udr, msg->type, ucsra);
    uart_send(udr, msg->length, ucsra);
    for(uint8_t i=0; i < msg->length; i++) {
        uart_send(udr, msg->payload[i], ucsra);
    }

    // 4. KLUCZOWY MOMENT: Czekaj a¿ ostatni bit fizycznie opuœci procesor
    // Pêtla krêci siê dopóki flaga Transmit Complete (TXC) jest zerem
    while(!(*ucsra & (1 << txc_bit)));

    // 5. Krótki margines na stop-bit i prze³¹cz na odbiór (DE=0)
    _delay_us(100);
    rs485_rx(link);
}


// --- Odbiór bajtu RS485 (buforowanie i CRC) ---
uint8_t rs485_receive_byte(uint8_t link, uint8_t b, mys_msg_t *msg){
    uint8_t *buf; uint8_t *idx;
    switch(link){
        case 1: buf=buf1; idx=&idx1; break;
        case 2: buf=buf2; idx=&idx2; break;
        case 3: buf=buf3; idx=&idx3; break;
        default: return 0;
    }

    if(*idx >= BUF_SIZE) { *idx=0; }
    buf[(*idx)++] = b;

    if(*idx<8) return 0;
    if(buf[0]!=START_BYTE){
        for(uint8_t i=1;i<*idx;i++) buf[i-1]=buf[i];
        (*idx)--;
        return 0;
    }


    uint8_t payload_len = buf[7];
    if(*idx < 8 + payload_len) return 0; // Czekamy tylko na START+HDR+PAYLOAD


    msg->sender = buf[1];
    msg->destination = buf[2];
    msg->child_id = buf[3];
    msg->command = buf[4];
    msg->ack = buf[5];
    msg->type = buf[6];
    msg->length = payload_len;
    for(uint8_t i=0;i<payload_len;i++) msg->payload[i]=buf[8+i];

    *idx=0;
    return 1;
}
