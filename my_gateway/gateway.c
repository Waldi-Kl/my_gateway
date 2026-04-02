/*
 * gateway.c
 *
 *  Created on: 18 lut 2026
 *      Author: kleczynskiw
 */
//
//#include <avr/io.h>
//#include <util/delay.h>
//#include <string.h>
//#include <stdio.h>
//#include <stdint.h>
//
//#define F_CPU 16000000UL
//#define BAUD_RS485 115200
//#define BAUD_HA 115200
//#define MYUBRR_RS485 (F_CPU/16/BAUD_RS485-1)
//#define MYUBRR_HA (F_CPU/16/BAUD_HA-1)
//
//#define START_BYTE 0x7E
//#define MAX_PAYLOAD 32
//
//#define DE_PIN 2  // DE/RE na Mega1280 (np. PortA0)
//
//typedef struct {
//    uint8_t sender;
//    uint8_t destination;
//    uint8_t child_id;
//    uint8_t command;
//    uint8_t ack;
//    uint8_t type;
//    uint8_t length;
//    uint8_t payload[MAX_PAYLOAD];
//} mys_msg_t;
//
//// --- DE/RE ---
//void rs485_tx(){ PORTA |= (1<<DE_PIN); _delay_us(10); }
//void rs485_rx(){ PORTA &= ~(1<<DE_PIN); _delay_us(10); }
//
//// --- UART ---
//void uart_init(volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
//               volatile uint8_t *ucsrb, volatile uint8_t *ucsrc, uint16_t ubrr)
//{
//    *ubrrh = (ubrr >> 8);
//    *ubrrl = (ubrr & 0xFF);
//    *ucsrb = (1<<RXEN0) | (1<<TXEN0);
//    *ucsrc = (1<<UCSZ01) | (1<<UCSZ00);
//}
//
//void uart_send(volatile uint8_t *udr, uint8_t data, volatile uint8_t *ucsra)
//{
//    while(!(*ucsra & (1<<UDRE0)));
//    *udr = data;
//}
//
//// --- CRC8 ---
//uint8_t crc8(uint8_t *data, uint8_t len){
//    uint8_t crc=0;
//    for(uint8_t i=0;i<len;i++) crc ^= data[i];
//    return crc;
//}
//
//// --- Serial API do HA ---
//void send_to_ha(mys_msg_t *msg){
//    char buf[96];
//    char payload[32] = {0};
//
//    if(msg->length == 1)
//        snprintf(payload,sizeof(payload),"%d",msg->payload[0]);
//    else if(msg->length == 2){
//        uint16_t v = (msg->payload[0]<<8)|msg->payload[1];
//        snprintf(payload,sizeof(payload),"%u",v);
//    }
//    else if(msg->length>2){
//        memcpy(payload,msg->payload,msg->length);
//        payload[msg->length]=0;
//    }
//
//    snprintf(buf,sizeof(buf),"%d;%d;%d;%d;%d;%s\n",
//             msg->sender,
//             msg->child_id,
//             msg->command,
//             msg->ack,
//             msg->type,
//             payload);
//
//    for(uint8_t i=0;i<strlen(buf);i++)
//        uart_send(&UDR0,buf[i],&UCSR0A);
//}
//
//// --- Wysyłanie na RS485 ---
//void rs485_send_msg(mys_msg_t *msg){
//    rs485_tx();
//
//    uart_send(&UDR1,START_BYTE,&UCSR1A);
//    uart_send(&UDR1,msg->sender,&UCSR1A);
//    uart_send(&UDR1,msg->destination,&UCSR1A);
//    uart_send(&UDR1,msg->child_id,&UCSR1A);
//    uart_send(&UDR1,msg->command,&UCSR1A);
//    uart_send(&UDR1,msg->ack,&UCSR1A);
//    uart_send(&UDR1,msg->type,&UCSR1A);
//    uart_send(&UDR1,msg->length,&UCSR1A);
//    for(uint8_t i=0;i<msg->length;i++)
//        uart_send(&UDR1,msg->payload[i],&UCSR1A);
//
//    uart_send(&UDR1,crc8(&msg->sender,7+msg->length),&UCSR1A);
//
//    rs485_rx();
//}
//
//// --- Parser z HA ---
//uint8_t parse_mys_line(char *line, mys_msg_t *msg){
//    uint8_t field=0;
//    char *start=line;
//
//    for(char *p=line;*p;p++){
//        if(*p==';'){
//            *p=0;
//            switch(field){
//                case 0: msg->sender=atoi(start); break;
//                case 1: msg->child_id=atoi(start); break;
//                case 2: msg->command=atoi(start); break;
//                case 3: msg->ack=atoi(start); break;
//                case 4: msg->type=atoi(start); break;
//            }
//            field++;
//            start=p+1;
//        }
//    }
//    strcpy((char*)msg->payload,start);
//    msg->length=strlen((char*)msg->payload);
//    return (field>=5);
//}
//
//// --- DE/RE init ---
//void init_de_re(){ DDRA |= (1<<DE_PIN); rs485_rx(); }
//
//// --- Main ---
//int main(void){
//    uart_init(&UBRR0H,&UBRR0L,&UCSR0B,&UCSR0C,MYUBRR_HA);   // Serial0 → HA
//    uart_init(&UBRR1H,&UBRR1L,&UCSR1B,&UCSR1C,MYUBRR_RS485); // Serial1 → RS485
//    init_de_re();
//
//    mys_msg_t ha_msg;
//    static char line[64]; static uint8_t idx=0;
//
//    while(1){
//        // --- Serial HA ---
//        if(UCSR0A & (1<<RXC0)){
//            char b=UDR0;
//            if(b=='\n'){
//                line[idx]=0;
//                if(parse_mys_line(line,&ha_msg))
//                    rs485_send_msg(&ha_msg);
//                idx=0;
//            } else if(idx<63) line[idx++]=b;
//        }
//
//        // --- TODO: odbiór RS485 i wysyłka do HA ---
//        // Buforowanie bajtów, sprawdzanie START_BYTE, CRC, konwersja → send_to_ha()
//    }
//}
//
