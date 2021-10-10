/*Essa atividade não existe no roteiro e foi criada para auxiliar a criacao da atividade 6*/

#define F_CPU 16000000UL
#define MAX_BUFFER 5


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile unsigned int i_msg = 0;
char msg_1[] = "1111111111111111111111111.\n\n";
char msg_2[] = "2222222222222222222222222.\n\n";
char msg_3[] = "3333333333333333333333333.\n\n";
char msg_undefined[] = "NNNNNNNNNNNNNNNNNNNNNNNNN. \n\n";

/******** Buffer Circular ********/
volatile unsigned int i = 0;
volatile unsigned int j = 0;
volatile unsigned int qnt_buffer = 0;

volatile unsigned char buffer_circular[MAX_BUFFER];
volatile unsigned char comando_atual = 0;

/******** UART ********/
unsigned char *p_ubrr0H;
unsigned char *p_ubrr0L;
unsigned char *p_ucsr0A;
unsigned char *p_ucsr0B;
unsigned char *p_ucsr0C;

unsigned char *p_udr0;

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
    if (!(str[i_msg - 1] == '\n' && str[i_msg] == '\n')) {
        *p_udr0 = str[i_msg];
        i_msg++;
    } else {
        *p_udr0 = str[i_msg];
        i_msg = 0;
        *p_ucsr0B &= ~0x20; // apos terminar de enviar a msg, desabilata interrupcao
    }
}

/************** Servicos do Buffer Circular **************/
void adiciona_buffer(int in) {
    if (qnt_buffer < MAX_BUFFER) {
        buffer_circular[i] = in;
        qnt_buffer++;
        i = adicionar_indice(i);
    }
}

volatile unsigned char pega_comando_do_buffer() {
    int output = 0;
    if (qnt_buffer > 0) {
        output = buffer_circular[j];
        buffer_circular[j] = 0;
        qnt_buffer--;
        j = adicionar_indice(j);
    }
    return output;
}

volatile unsigned int adicionar_indice(int k) {
    if (k == (MAX_BUFFER-1)) {
        return 0;
    } else {
        return k + 1;
    }
}

ISR(USART_UDRE_vect) {
        switch (comando_atual) {
            case '1':
                print_msg(msg_1);
            //controle do led
            break;
            case '2':
                print_msg(msg_2);
            //controle do led
            break;
            case '3':
                print_msg(msg_3);
            //controle do led
            break;
            default:
                print_msg(msg_undefined);
            break;
        }
}

ISR(USART_RX_vect) {
        adiciona_buffer(*p_udr0);
}

int main() {
    config();
    while (1) {
        if (qnt_buffer > 0) {
            if ( (*p_ucsr0B & 0x20) != 0x20) { //uma nova interrupcao sera pegad o buffer apenas se a ultima tiver terminado
                comando_atual = pega_comando_do_buffer();
                *p_ucsr0B |= 0x20; //quando tiver algo no buffer, habilita-se a interrupção de buffer vazio
            }
        }
    }
}