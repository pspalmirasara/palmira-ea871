/*Essa atividade não existe no roteiro e foi criada para auxiliar a criacao da atividade 6*/

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile unsigned char i_msg = 0;
char msg_1 [] = "Essa eh a mensagem de numero 1 (um).\n\n";
char msg_2 [] = "Essa eh a mensagem de numero 2 (dois).\n\n";
char msg_3 [] = "Essa eh a mensagem de numero 3 (tres).\n\n";
char msg_undefined [] = "Nao existe mensagem para este comando. \n\n";


volatile unsigned char i = 0;
//volatile -> usado para ter certeza que a variavel que eh manipulada na interrupcao e na main()
//nao ficara com valor inconsistente por possiveis otimizacoes do compilador
volatile unsigned char input = 0;
volatile unsigned char chegou = 0;

unsigned char *p_ubrr0H;
unsigned char *p_ubrr0L;
unsigned char *p_ucsr0A;
unsigned char *p_ucsr0B;
unsigned char *p_ucsr0C;

unsigned char *p_udr0;

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
    //  1       0      0      1     1     0      X     X
    *p_ucsr0B = 0x98;
    //inicialmente, a interrupcao de buffer vazio esta desabilitada para o programa nao chama-la logo de inicio

    // UCSRnC – USART Control and Status Register n C
    // UMSELn1 UMSELn0 UPMn1 UPMn0 USBSn UCSZn1 UCSZn0 UCPOLn
    //    0       0      0     0     1      1     1      0
    *p_ucsr0C = 0x0E;

    /*Habilita interrupções globais*/
    sei();
}

void print_msg(char *str) {
    if (!(str[i_msg-1] == '\n' &&  str[i_msg] == '\n')) {
        *p_udr0 = str[i_msg];
        i_msg++;
    } else {
        *p_udr0 = str[i_msg];
        i_msg = 0;
        *p_ucsr0B &= ~0x20; // apos terminar de enviar a msg, desabilata interrupcao
    }
}

ISR(USART_UDRE_vect) {
        switch(input) {
            case '1':
                print_msg(msg_1);
                break;
            case '2':
                 print_msg(msg_2);
                 break;
            case '3':
                print_msg(msg_3);
                break;
            default:
                print_msg(msg_undefined);
                break;
        }

        chegou = 0;
}

ISR(USART_RX_vect) {
        input = *p_udr0;
        chegou = 1;
}

int main() {
    config();
    while (1) {
        if (chegou == 1) {
            *p_ucsr0B |= 0x20; //quando chegar alguma coisa, habilita-se a interrupção de buffer vazio
        }
    }
}