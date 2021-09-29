#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


/****** Settando Interrupções ******/
/* Ponteiro para o registrador PCICR */
unsigned char *p_pcicr;

/* Ponteiro para o registrador PCMSK2 */
unsigned char *p_pcmsk2;

/* Ponteiro para o registrador EICRA*/
unsigned char *p_eicra;

/* Ponteiro para o registrador EIMSK */
unsigned char *p_eimsk;

/****** END ******/

/* Ponteiro para o registrador MCUCR */
unsigned char *p_mcucr;

/* Ponteiros para os registradores do PORTC */
unsigned char *p_portc;
unsigned char *p_pinc;
unsigned char *p_ddrc;

/* Ponteiros para os registradores do PORTD */
unsigned char *p_portd;
unsigned char *p_pind;
unsigned char *p_ddrd;

/* Variavel que guarda o estado do botao */
unsigned char botao = 0;

/* Variavel que guarda o estado do led decimal */
unsigned char led = 0;

/* Inicializacoes dos perifericos envolvidos no sistema */
void configuracoes_iniciais() {

    cli();

    p_pcicr = (unsigned char *) 0x68;
    p_pcmsk2 = (unsigned char *) 0x6D;

    p_eicra = (unsigned char *) 0x69;
    p_eimsk = (unsigned char *) 0x3D;

    /* PORTC */
    p_pinc = (unsigned char *) 0x26;
    p_ddrc = (unsigned char *) 0x27;
    p_portc = (unsigned char *) 0x28;
    /* PORTD */
    p_pind = (unsigned char *) 0x29;
    p_ddrd = (unsigned char *) 0x2A;
    p_portd = (unsigned char *) 0x2B;



    // PC0 como saida (LED)
    *p_ddrc |= 0x01;
    *p_portc &= 0XFE;

    //Configuracoes da PCINT18
    // PD2 como entrada
    *p_ddrd &= ~0x04;

    // Setta o bit PCIE2: habilita a mascar do grupo associado o PCINT18
    *p_pcicr |= 0x04;

    // Setta o bit PCIE18: habilita a mascara de PCINT18
    *p_pcmsk2 |= 0x04;

    sei();
}

ISR(PCINT2_vect) {
    int estado_anterior_botao = *p_pind & 0x04;


    if (estado_anterior_botao == 0) {
        if (led == 0) {
            led = 1;
            *p_portc |= 1;
        } else {
            led = 0;
            *p_portc &= 0xFE;
        }
    }
}

int main(void) {
    configuracoes_iniciais();
    /* Loop infinito */
    while (1);

    return 0;
}