/*
 * EA871 - Turma A
 * Palmira Sara Aranovich Florentino RA 185306
 * Atividade 8
 * tinkercard: https://www.tinkercad.com/things/2sfK7cmA9P8
 */

#define F_CPU 16000000UL
#define VALUE_OCR0A 251

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/******** Temporizador ********/
volatile unsigned int fade_out = 1;
volatile unsigned int contador_segundos = 0; //aqui, consideraremos que 1seg = 1004 interrupcoes
volatile unsigned int contador_duty_cycle = 0;

void config() {
    /*Desabilita interrupções globais*/
    cli();

    /* Especificacoes do temporizador */

    /*
     * (Contas)
     * Tpwm = (OCR0A + 1) P/fcpu
     * Tpwm = (OCR0A + 1) * 64/16000000
     * Com Tpwm = 0.001s (1ms)
     * 0.001 * 16000000/64 = (OCR0A + 1)
     * 250+1 = OCR0A ---> OCR0A = 251
     *
     * Para realizar o fade-in e o fade-out, eh preciso incrementar e decrementar
     * de uma em uma a unidade do duty cycle
     * Logo, 1s / 251 =~ 0.004s
     * Para levar em conta o arredondamento, 251*0.004s = 1,004s.
     * Logo, durante o programa, 1s sera considerado 1004 ciclos.
     */

    OCR0A = VALUE_OCR0A;
    OCR0B = VALUE_OCR0A; //comeca com duty cycle de 100%

    // TIMSK0 – Timer/Counter Interrupt Mask Register
    // – – – – – OCIE0B OCIE0A TOIE0
    //   0         0      0      1
    //TOIE0: contagem resetta quando a comparação eh zerada
    TIMSK0 = 0x01;

    // TCCR0B – Timer/Counter Control Register B
    // FOC0A FOC0B – – WGM02 CS02 CS01 CS00
    // 0      0   - -   1    0     1   1
    // CS02, CS01 e CS00 são o valor do prescaler, que eh 64: 011 (retirado do manual)
    TCCR0B = 0x0B;

    // TCCR0A – Timer/Counter Control Register A
    // COM0A1 COM0A0 COM0B1 COM0B0 – – WGM01 WGM00
    //  0      0       1     0    - -    1    1
    // WGM02, WGM01 E WGM00: setta o modo (111 para Fast PMW com TOP=OCRA0)
    // COM0B1: zera OC0B ao igualar comparacao (OC0B eh onde saira a forma de onda, no pino 5)
    TCCR0A = 0x23;

    /* Especificacoes dos LED */
    //The Port B Data Register
    //PORTB5 = pino 13 (LED embutido)
    DDRB |= 0x20; // Configura o LED como saida
    PORTB &= ~(0x20);  // LED começa desligado (programa comeca em fade-out)

    //The Port D Data Register
    //PORTD5 = pino 5 (LED externo)
    DDRD |= 0x20; // Configura o LED como saida
    // nao eh preciso determinar o nivel desse LED pois o que esta nessa saida eh o OCB0b

    /*Habilita interrupções globais*/
    sei();
}

/************** Interrupcoes: BEGIN **************/
ISR(TIMER0_OVF_vect) {
        //contador dos segundos
        contador_segundos++;
        if (contador_segundos == 1004) {
            contador_segundos = 0;
            if (fade_out == 1) {
                // muda para fade_in
                fade_out = 0;
                PORTB |= 0x20; //fade_in: LED embutido ligado
            } else {
                // muda para fade_out
                fade_out = 1;
                PORTB &= ~(0x20); //fade_out: LED embutido desligado
            }
        }

        //a cada 4ms, incrementa ou decrementa, dependendo se esta em fade-in ou fade-out
        contador_duty_cycle++;
        if (contador_duty_cycle == 4) {
            contador_duty_cycle = 0;
            if (fade_out == 1) {
                //em fade-out, decrementa
                OCR0B = OCR0B - 1;
            } else {
                //em fade-in, incrementa
                OCR0B = OCR0B + 1;
            }
        }
}
/************** Interrupcoes: END **************/

int main() {
    config();
    while (1) {
        _delay_ms(1);
    }
}