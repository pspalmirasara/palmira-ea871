#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

char msg_TXC0[] = "Trasmissao serial utilizando a interrupcao USART Transmit Complete. \n\n";
int i = 0;

unsigned char * p_ubrr0H;
unsigned char * p_ubrr0L;
unsigned char * p_ucsr0A;
unsigned char * p_ucsr0B;
unsigned char * p_ucsr0C;

unsigned char * p_udr0;

/* Ponteiro para o registrador MCUCR */
unsigned char *p_mcucr;

/* Ponteiros para os registradores do PORTB */
unsigned char *p_portB;
unsigned char *p_pinB;
unsigned char *p_ddrB;


void config() {
    /*Desabilita interrupções globais*/
    cli();

    /*Enderecos do UART*/
    p_ubrr0H = (unsigned char *) 0xC5;
    p_ubrr0L = (unsigned char *) 0xC4;
    p_ucsr0A = (unsigned char *) 0xC0;
    p_ucsr0B = (unsigned char *) 0xC1;
    p_ucsr0C = (unsigned char *) 0xC2;

    /*Registrador que recebera o dado a ser transmitido*/
    p_udr0 = (unsigned char *) 0xC6;

    /*Especificacoes da atividade*/
    /*Registradores que configuram o Baud rate*/
    /*especificado na atividade: 15.2k com -3.5% de erro*/
    *p_ubrr0H = 0;
    *p_ubrr0L = 16;

    // UCSRnA – USART Control and Status Register n A
    // RXCn TXCn UDREn FEn DORn UPEn U2Xn MPCMn
    //  X    X     X    X    X   X    1     0
    *p_ucsr0A = 0;

    // UCSRnB – USART Control and Status Register n B
    // RXCIEn TXCIEn UDRIEn RXENn TXENn UCSZn2 RXB8n TXB8n
    //  0       1      0      0     1     0      X     X
    *p_ucsr0B = 0x48;

    // UCSRnC – USART Control and Status Register n C
    // UMSELn1 UMSELn0 UPMn1 UPMn0 USBSn UCSZn1 UCSZn0 UCPOLn
    //    0       0      0     0     1      1     1      0
    *p_ucsr0C = 0x0E;

    sei();
}

ISR(USART_TX_vect) {
    if (!(msg_TXC0[i-1] == '\n' &&  msg_TXC0[i] == '\n')) {
        *p_udr0 = msg_TXC0[i];
        i++;
    } else {
        _delay_ms(500*4);
        *p_udr0 = msg_TXC0[i];
        i = 0;
    }
}

int main () {
    config();
    i = 1;
    *p_udr0 = msg_TXC0[0];
    while(1) {

    }
}
