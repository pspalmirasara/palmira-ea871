#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/*
 * EA871 - Turma A
 * Palmira Sara Aranovich Florentino RA 185306
 * Atividade 6
 * tinkercard: https://www.tinkercad.com/things/2AjSwK9hi7X-atividade5aula/
 */

/******** Buffer Circular ********/
volatile unsigned char buffer_circular [4];
volatile unsigned char i = 0;
volatile unsigned char j = 0;
volatile unsigned char qnt_buffer = 0;

/******** Variaveis ********/
//volatile -> usado para ter certeza que a variavel que eh manipulada na interrupcao e na main()
//nao ficara com valor inconsistente por possiveis otimizacoes do compilador
volatile unsigned char input = 0;
volatile unsigned char chegou = 0;

/* Ponteiros para os registradores de configuracao da UART */
unsigned char *p_ubrr0H;
unsigned char *p_ubrr0L;
unsigned char *p_ucsr0A;
unsigned char *p_ucsr0B;
unsigned char *p_ucsr0C;

unsigned char *p_udr0;

/* Ponteiros para os registrados do PORTC*/
unsigned char *p_portC;
unsigned char *p_pinC;
unsigned char *p_ddrC;

void config() {
    /*Desabilita interrupções globais*/
    cli();

    /*Endereco da PORTC*/
    p_pinC = (unsigned char *) 0x26;
    p_ddrC = (unsigned char *) 0x27;
    p_portC = (unsigned char *) 0x28;

    /*Enderecos do UART*/
    p_ubrr0H = (unsigned char *) 0xC5;
    p_ubrr0L = (unsigned char *) 0xC4;
    p_ucsr0A = (unsigned char *) 0xC0;
    p_ucsr0B = (unsigned char *) 0xC1;
    p_ucsr0C = (unsigned char *) 0xC2;

    /*Registrador que recebera o dado a ser transmitido e o dado que chegar da transmissao*/
    p_udr0 = (unsigned char *) 0xC6;

    /******** PortC ********/
    /*todo: nao lembro o que precisa fazer para configurar led*/


    /******** Especificacoes da atividade - UART ********/

    /*todo: explicar o baund rate e a velocidade dupla*/
    *p_ubrr0H = 0;
    *p_ubrr0L = 16;

    /*UCSR0A: configuracoes*/
    // UCSRnA – USART Control and Status Register n A
    // RXCn TXCn UDREn FEn DORn UPEn U2Xn MPCMn
    //  X    X     X    X    X   X    1     0
    *p_ucsr0A = 0;

    /*UCSR0B: configuracoes*/
    // UCSRnB – USART Control and Status Register n B
    // RXCIEn TXCIEn UDRIEn RXENn TXENn UCSZn2 RXB8n TXB8n
    //  1       0      0      1     1     0      X     X
    *p_ucsr0B = 0x98; //inicialmente, a interrupcao de buffer vazio
    // esta desabilitada para o programa nao chama-la logo de inicio

    /*UCSR0B: configuracoes*/
    // UCSRnC – USART Control and Status Register n C
    // UMSELn1 UMSELn0 UPMn1 UPMn0 USBSn UCSZn1 UCSZn0 UCPOLn
    //    0       0      0     0     1      1     1      0
    *p_ucsr0C = 0x0E;

    /*Habilita interrupções globais*/
    sei();
}

/************** Servicos do Buffer Circular **************/
void adiciona_buffer(int in) {
    *buffer_circular[i] = in;
    qnt_buffer++;
    i++;
}

void retira_buffer() {
    if (qnt_buffer > 0) {
        *buffer_circular[j] = 0;
        qnt_buffer--;
        j++;
    }
}

void controla_buffer(int in) {
    if (qnt_buffer < 5) {
        adiciona_buffer(int in);
    } else {
        retira_buffer();
    }
}

/************** Interrupcoes **************/
/*Interrupcao disparada quando o buffer para enviar esta vazio*/
ISR(USART_UDRE_vect) {
        /*Essa interrupcao sera usada para printar a mensagem na tela do usuario*/
        *p_udr0 = input;
        chegou = 0;
        *p_ucsr0B &= ~0x20; //desabilita a interrupcao de buffer vazio
}

/*Interrupcao disparada quando recebe uma informacao*/
ISR(USART_RX_vect) {
        input = *p_udr0;
        adiciona_buffer(input);
        chegou = 1;
}

/************** Main **************/
int main() {
    config();
    while (1) {
        if (chegou == 1) {
            *p_ucsr0B |= 0x20; //quando chegar alguma coisa, habilita-se a interrupção de buffer vazio


        }
    }
}
