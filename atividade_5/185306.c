#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/*
 * EA871 - Turma A
 * Palmira Sara Aranovich Florentino RA 185306
 * Atividade 5
 * tinkercard: https://www.tinkercad.com/things/2AjSwK9hi7X-atividade5aula/
 */

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

    /*Endereco da PORTB*/
    p_pinB = (unsigned char *) 0x23;
    p_ddrB = (unsigned char *) 0x24;
    p_portB = (unsigned char *) 0x25;

    /*Endereco para a MCUCR*/
    p_mcucr = (unsigned char *) 0x55;


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
    *p_ubrr0L = 8;

    /*UCSR0A: configuracoes*/
    /*Alem das configuracoes, os 3 bits mais significativos desse registrador
     * sao flags com informacoes do buffer.
     * Sera utilizada nesse laboratorio a UDRE0 (3o bit mais significativo,
     * que quando esta ativada, sabemos que o buffer esta vazio e pode ser
     * escrito*/

    /* Velocidade de transmissão normal (0): segundo bit menos significativo
     * Modo de comunicação multiprocessador (0): primeiro bit menos significativo
     * e o restante dos bits sao don't care*/
    *p_ucsr0A = 0;

    /*UCSR0B: configuracoes*/
     /* os tres bits mais significativos sao relacionados a interrupção, que serao desativadas (0)
     * desligar receptor (0): 5o bit
     * ligar o transmissor (1): 4o bit menos significativo
     * 8 bits por frame (0): 3o bit menos significativo (uma das três configuracoes que tem que ser feita)
     * os ultimos dois bits, como nao usamos 9 bits, eles nao serao settados*/
    *p_ucsr0B = 8;

    /*UCSR0B: configuracoes*/
    /* Modo assincrono: bit 7 e 6 = 0
     * Paridade par: bit 5 e 4 = 1 e 0 respectivamente
     * 2 bits de parada: bit 3 = 1
     * numero de bits por frame (8): bit 2 e 1 = 1
     * bit menos significativo precisa ser 0 para modo assincrono
     */
    *p_ucsr0C = 0x2E;

    /*Ativando os resistores de pull-up:
     * O bit 4 do MCUCR deve ser 0 para poderem ser
     * ativados os resistores de pull-up em cada PORT
     */
    *p_mcucr &= 0xEF;
    /*Ativar o pull up na entrada do botao (PB1: 2o menos significativo da PORTB)*/
    *p_portB |= 0x02;
}

void printMsg (char *str) {
    int i = 0;
    while ( !(str[i-1] == '\n' && str[i] == '\n') ) {
        *p_udr0 = str[i];
        //Esperando o buffer ficar vazio ((*p_ucsr0A & 0x20)==1))
        while((*p_ucsr0A & 0x20)==0);
        i++;
    }
}

int main () {

    char msg_np[] = "Transmissao serial utilizando a USART: verificacao do termino da transmissao por varredura. O botao nao esta pressionado.\n\n";
    char msg_p[] = "Transmissao serial utilizando a USART: verificacao do termino da transmissao por varredura. O botao esta pressionado.\n\n";

    config();

    while(1) {

        if ((*p_pinB & 0x02) == 2) { //botao solto
            printMsg(msg_p);
        } else { //botao pressionado
            printMsg(msg_np);
        }

        _delay_ms(500);
    }
}