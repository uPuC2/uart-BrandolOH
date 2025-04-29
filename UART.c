#include <avr/io.h>
#include <stdint.h>

#define F_OSC 16000000UL
#define MAX_STR_LEN 128

// Registros por UART (0-3)
volatile uint8_t *regUBRRH[4] = {&UBRR0H, &UBRR1H, &UBRR2H, &UBRR3H};
volatile uint8_t *regUBRRL[4] = {&UBRR0L, &UBRR1L, &UBRR2L, &UBRR3L};
volatile uint8_t *regUSCRA[4] = {&UCSR0A, &UCSR1A, &UCSR2A, &UCSR3A};
volatile uint8_t *regUCSRB[4] = {&UCSR0B, &UCSR1B, &UCSR2B, &UCSR3B};
volatile uint8_t *regUCSRC[4] = {&UCSR0C, &UCSR1C, &UCSR2C, &UCSR3C};
volatile uint8_t *regUDR[4]   = {&UDR0, &UDR1, &UDR2, &UDR3};

// ========== FUNCIONES DE INICIALIZACIÓN ==========

void UART_Ini(uint8_t com, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop) {
    // Validación de parámetros
    if(com > 3 || size < 5 || size > 8 || parity > 2 || stop < 1 || stop > 2) {
        return;
    }

    // Configuración del baud rate
    uint32_t ubrrValue = F_OSC / (16UL * baudrate) - 1;
    *regUBRRH[com] = (uint8_t)(ubrrValue >> 8);
    *regUBRRL[com] = (uint8_t)ubrrValue;

    // Configuración del formato del frame
    *regUCSRC[com] = ((size - 5) << UCSZ00); // Tamaño de datos (sin URSEL0)
    
    // Configuración de paridad
    if(parity == 1) {
        *regUCSRC[com] |= (1 << UPM01) | (1 << UPM00); // Odd parity
    } else if(parity == 2) {
        *regUCSRC[com] |= (1 << UPM01);               // Even parity
    }
    
    // Bit de stop
    if(stop == 2) {
        *regUCSRC[com] |= (1 << USBS0);
    }
    
    // Habilitar transmisor y receptor (sin interrupciones)
    *regUCSRB[com] = (1 << RXEN0) | (1 << TXEN0);
}

// ========== FUNCIONES DE TRANSMISIÓN ==========

void UART_putchar(uint8_t com, char data) {
    // Esperar hasta que el buffer de transmisión esté vacío
    while(!(*regUSCRA[com] & (1 << UDRE0)));
    *regUDR[com] = data;
}

void UART_puts(uint8_t com, char *str) {
    while(*str) {
        UART_putchar(com, *str++);
    }
}

// ========== FUNCIONES DE RECEPCIÓN ==========

uint8_t UART_available(uint8_t com) {
    if(com > 3) return 0;
    return (*regUSCRA[com] & (1 << RXC0)) ? 1 : 0;
}

char UART_getchar(uint8_t com) {
    // Esperar hasta que haya datos disponibles
    while(!UART_available(com));
    return *regUDR[com];
}

void UART_gets(uint8_t com, char *str) {
    char c;
    char *start = str;
	uint8_t count = 0;
    while(1) {
        c = UART_getchar(com);
		
        if(c == 13) { // Si presionan Enter (ASCII 13)
            UART_putchar(com, '\n');  // Imprime salto de línea
            break;
        } else if(c == 8) { // Manejo de Backspace
            if(str > start) {
                str--;
                UART_putchar(com, 8);
                UART_putchar(com, ' ');
                UART_putchar(com, 8);
				str[--count];
            }
        } else if(c >= 32 && c <= 126  && count < MAX_STR_LEN) { // Solo caracteres imprimibles
            *str++ = c;
            UART_putchar(com, c); // Eco
			str[++count];
        }
    }
    *str = '\0';
}

// ========== FUNCIONES ANSI ==========

void UART_clrscr(uint8_t com) {
    UART_puts(com, "\033[2J");  // Limpiar pantalla
    UART_puts(com, "\033[H");   // Mover cursor al inicio
}

void UART_setColor(uint8_t com, uint8_t color) {
    UART_putchar(com, '\033');
    UART_putchar(com, '[');
    
    if(color >= 10) {
        UART_putchar(com, '0' + color/10);
    }
    UART_putchar(com, '0' + color%10);
    UART_putchar(com, 'm');
}

void UART_gotoxy(uint8_t com, uint8_t x, uint8_t y) {
    UART_putchar(com, '\033');
    UART_putchar(com, '[');
    
    // Coordenada Y (fila)
    if(y >= 10) {
        UART_putchar(com, '0' + y/10);
    }
    UART_putchar(com, '0' + y%10);
    
    UART_putchar(com, ';');
    
    // Coordenada X (columna)
    if(x >= 10) {
        UART_putchar(com, '0' + x/10);
    }
    UART_putchar(com, '0' + x%10);
    
    UART_putchar(com, 'H');
}

// ========== FUNCIONES DE CONVERSIÓN ==========

void itoa(uint16_t number, char* str, uint8_t base) {
    uint8_t i = 0;
    uint8_t digits = 0;
    char temp[16];
    
    // Manejo del caso especial 0
    if(number == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    // Convertir dígitos en orden inverso
    while(number > 0) {
        uint8_t digit = number % base;
        temp[digits++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        number /= base;
    }
    
    // Invertir los dígitos
    for(i = 0; i < digits; i++) {
        str[i] = temp[digits - 1 - i];
    }
    str[digits] = '\0';
}

uint16_t atoi(char *str) {
    uint16_t num = 0;
    
    while(*str >= '0' && *str <= '9') {
        num = num * 10 + (*str - '0');
        str++;
    }
    
    return num;
}
