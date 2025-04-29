#ifndef UART_H_
#define UART_H_

#include <stdint.h>

// ========== CONSTANTES PARA COLORES ANSI ==========
#define COLOR_RESET   0
#define COLOR_RED    31
#define COLOR_GREEN  32
#define COLOR_YELLOW 33
#define COLOR_BLUE   34
#define COLOR_PURPLE 35
#define COLOR_CYAN   36

// ========== PROTOTIPOS DE FUNCIONES ==========

void UART_Ini(uint8_t com, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop);

// ----- Transmisión -----

void UART_puts(uint8_t com, char *str);
void UART_putchar(uint8_t com, char data);

// ----- Recepción -----
uint8_t UART_available(uint8_t com);
char UART_getchar(uint8_t com);
void UART_gets(uint8_t com, char *str);

// ----- Secuencias ANSI -----
void UART_clrscr(uint8_t com);
void UART_setColor(uint8_t com, uint8_t color);
void UART_gotoxy(uint8_t com, uint8_t x, uint8_t y);

// ----- Funciones de conversión -----
void itoa(uint16_t number, char* str, uint8_t base);
uint16_t atoi(char *str);

#endif /* UART_H_ */
