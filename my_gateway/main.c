/*
 * main.c
 *
 *  Created on: 18 lut 2026
 *      Author: kleczynskiw
 */

#include "rs485.h"
#include "serial_api.h"
#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL
#define BAUD_HA 38400
#define BAUD_RS485 38400
#define MYUBRR_HA (F_CPU/16/BAUD_HA-1)
#define MYUBRR_RS485 (F_CPU/16/BAUD_RS485-1)

void uart0_send_char(char c) {
	while (!(UCSR0A & (1 << UDRE0)))
		;  // czekaj a¿ bufor pusty
	UDR0 = c;
}

void uart0_send_string(const char *s) {
	while (*s) {
		uart0_send_char(*s++);
	}
	uart0_send_char('\r'); // Carriage Return
	uart0_send_char('\n'); // Line Feed
}

void uart1_send_char(char c) {
	while (!(UCSR1A & (1 << UDRE1)))
		;  // czekaj a¿ bufor nadajnika pusty
	UDR1 = c;                          // wyœlij znak
}
void uart1_send_string(const char *s) {
	while (*s) {
		uart1_send_char(*s++);
	}
}

void uart_init(volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
		volatile uint8_t *ucsrb, volatile uint8_t *ucsrc, uint16_t ubrr) {
	*ubrrh = (ubrr >> 8);
	*ubrrl = (ubrr & 0xFF);
	*ucsrb = (1 << RXEN0) | (1 << TXEN0);
	*ucsrc = (1 << UCSZ01) | (1 << UCSZ00);
}

void presentation(void) {
	_delay_ms(5000);

	uart0_send_string("255;255;3;0;2;2.0");
	_delay_ms(1000);

	uart0_send_string("0;255;0;0;17;2.0");
	_delay_ms(500);

	uart0_send_string("0;255;3;0;11;Bramka RS485");
	_delay_ms(500);
	uart0_send_string("0;255;3;0;12;2.0");
	_delay_ms(500);

}

int main(void) {
	uart_init(&UBRR0H, &UBRR0L, &UCSR0B, &UCSR0C, MYUBRR_HA);   // Serial HA
	uart_init(&UBRR1H, &UBRR1L, &UCSR1B, &UCSR1C, MYUBRR_RS485); // Link1
	uart_init(&UBRR2H, &UBRR2L, &UCSR2B, &UCSR2C, MYUBRR_RS485); // Link2
	uart_init(&UBRR3H, &UBRR3L, &UCSR3B, &UCSR3C, MYUBRR_RS485); // Link3

//    uart0_send_string("0;255;3;0;14;Gateway Startup Complete\r\n");

	presentation();

	rs485_init();

	mys_msg_t msg;
	static char ha_line[64];
	static uint8_t ha_idx = 0;


	while (1) {
		// --- Serial HA ---
//		if (UCSR0A & (1 << RXC0)) {
//			char b = UDR0;
//			if (b == '\n') {
//				ha_line[ha_idx] = 0;
//				// usuñ mo¿liwe \r
//				if (ha_idx > 0 && ha_line[ha_idx - 1] == '\r')
//					ha_line[ha_idx - 1] = 0;
//				if (parse_mys_line(ha_line, &msg))
//					uart0_send_string("PARSE OK");
//				rs485_send_msg(1, &msg); // wysy³amy na pierwszy link
//				ha_idx = 0;
//			} else if (ha_idx < 63)
//				ha_line[ha_idx++] = b;
//		}
		if (UCSR0A & (1 << RXC0)) {
			char b = UDR0;

			// --- echo znaków od razu ---
//		    uart0_send_char(b);  // wyœlij od razu znak do terminala

			// --- zapis do bufora dla ca³ej linii ---
			if (ha_idx < 63)
				ha_line[ha_idx++] = b;

			// --- jeœli koniec linii (\n) ---
			if (b == '\n') {
				ha_line[ha_idx] = 0;
				if (ha_idx > 0 && ha_line[ha_idx - 1] == '\r')
					ha_line[ha_idx - 1] = 0;

				// --- parsowanie i potwierdzenie ---
				if (parse_mys_line(ha_line, &msg)) {
//					uart0_send_string("PARSE OK"); // teraz zobaczysz w terminalu
//					uart1_send_string(ha_line);
//					uart1_send_string("\r\n");
//					uart1_send_string("END ter2");
					rs485_send_msg(1, &msg);       // wysy³amy na pierwszy link
				}
//				else {
//					uart0_send_string("PARSE FAIL"); // jeœli linia niepoprawna
//				}

				ha_idx = 0; // reset indeksu bufora
			}
		}

		// --- RS485 Link1 ---
		if (UCSR1A & (1 << RXC1)) {
			uint8_t b = UDR1;
			if (rs485_receive_byte(1, b, &msg))
				send_to_ha(&msg);
		}
		// --- RS485 Link2 ---
		if (UCSR2A & (1 << RXC2)) {
			uint8_t b = UDR2;
			if (rs485_receive_byte(2, b, &msg))
				send_to_ha(&msg);
		}
		// --- RS485 Link3 ---
		if (UCSR3A & (1 << RXC3)) {
			uint8_t b = UDR3;
			if (rs485_receive_byte(3, b, &msg))
				send_to_ha(&msg);
		}
	}
}
