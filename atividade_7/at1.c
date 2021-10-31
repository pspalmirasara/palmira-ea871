/*
 * EA871 - Turma A
 * Palmira Sara Aranovich Florentino RA 185306
 * Atividade 7
 * tinkercard: https://www.tinkercad.com/things/4iS2KJd0pr5-185306-ea871-2s2020-atividade-6/
 */

#define F_CPU 16000000UL
#define MAX_BUFFER 5
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/******** LED ********/
// Porta 12 e 13 do arduino
unsigned char *p_portB;
unsigned char *p_ddrB;

/******** Temporizador ********/
unsigned char *p_tccr0a;
unsigned char *p_tccr0b;
unsigned char *p_timsk0;

volatile unsigned  int contador = 0;
volatile unsigned char estado = 0;


void config() {
    /*Desabilita interrupções globais*/
    cli();

    /*Enderecos dos temporizadores*/
    p_tccr0a = (unsigned char *) 0x44;
    p_tccr0b = (unsigned char *) 0x45;
    p_timsk0 = (unsigned char *) 0x6E;

    /*Endereco da PORTB*/
    p_ddrB = (unsigned char *) 0x24;
    p_portB = (unsigned char *) 0x25;

    // TCCR0A – Timer/Counter Control Register A
    // COM0A1 COM0A0 COM0B1 COM0B0 – – WGM01 WGM00
    //  0      0       0     0    - -    0    0
    *p_tccr0a = 0;

    // TCCR0B – Timer/Counter Control Register B
    // FOC0A FOC0B – – WGM02 CS02 CS01 CS00
    // 0      0   - -   0    0     1   0
    *p_tccr0b = 2;

    // TIMSK0 – Timer/Counter Interrupt Mask Register
    // – – – – – OCIE0B OCIE0A TOIE0
    //   0         0      0      1
    *p_timsk0 = 1;

    *p_ddrB |= 1<<5;
    *p_portB &= ~(1<<5);

    //The Port B Data Register
    //– PORTB6 PORTB5 PORTB4 PORTB3 PORTB2 PORTB1 PORTB0
    //PORTC5: LED de baixo, PORTC4: LED do meio, PORTC3: LED de cima
    //*p_portC = 0; // Todos os LEDs começam desligados
    //*p_ddrC |= 0x38; // Configura os LEDs como saida

    /*Habilita interrupções globais*/
    sei();
}

/************** Interrupcoes: BEGIN **************/
ISR(TIMER0_OVF_vect) {
    contador++;
    if (contador >= 2000) {
        contador = 0;
        if (estado == 0) {
            *p_portB |= (1<<5);
            estado = 1;
        } else {
            *p_portB &= ~(1<<5);
            estado = 0;
        }
    }
}
/************** Interrupcoes: END **************/

int main() {
    config();
    while (1) {
    }
}