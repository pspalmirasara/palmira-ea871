//TODO: colocar as especificacoes da atividade 6
//TODO: servicos do LED

#define F_CPU 16000000UL
#define MAX_BUFFER 5
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/******** LED ********/
unsigned char *p_portC;
unsigned char *p_pinC;
unsigned char *p_ddrC;
volatile unsigned int led_c = 0;
volatile unsigned int led_m = 0;
volatile unsigned int led_b = 0;

/******** Print ********/
volatile unsigned int i_msg = 0;
char msg_1[] = "1111111111111111111111111.\n\n";
char msg_2[] = "2222222222222222222222222.\n\n";
char msg_3[] = "3333333333333333333333333.\n\n";
char msg_undefined[] = "NNNNNNNNNNNNNNNNNNNNNNNNN. \n\n";
char msg_vazio[] = "Vazio! \n\n";
volatile unsigned int esta_printando = 0;
volatile unsigned int cod_msg = 0; //ENUM: 1, 2, 3, 4: undefined, 5: vazio

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

    /*Endereco da PORTC*/
    p_pinC = (unsigned char *) 0x26;
    p_ddrC = (unsigned char *) 0x27;
    p_portC = (unsigned char *) 0x28;

    //The Port C Data Register
    //– PORTC6 PORTC5 PORTC4 PORTC3 PORTC2 PORTC1 PORTC0
    //PORTC5: LED baixo, PORTC4: LED meio, PORTC3: LED cima
    *p_portC |= 0x00;

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
    //  1       1      0      1     1     0      X     X
    *p_ucsr0B = 0xD8;
    //inicialmente, a interrupcao de buffer vazio esta desabilitada para o programa nao chama-la logo de inicio

    // UCSRnC – USART Control and Status Register n C
    // UMSELn1 UMSELn0 UPMn1 UPMn0 USBSn UCSZn1 UCSZn0 UCPOLn
    //    0       0      0     0     1      1     1      0
    *p_ucsr0C = 0x0E;

    /*Habilita interrupções globais*/
    sei();
}

/************** Servicos do LED: BEGIN **************/
//c para o led de cima, m para o do meio e b para o de baixo
void liga_led(char led) {
    if (led == 'c') {
        *p_portC |= 0x08;
        led_c = 1;
    }
    if (led == 'm') {
        *p_portC |= 0x10;
        led_m = 1;
    }
    if (led == 'b') {
        *p_portC |= 0x20;
        led_b = 1;
    }
}

void apaga_led(char led) {
    if (led == 'c') {
        *p_portC &= ~0x08;
        led_c = 0;
    }
    if (led == 'm') {
        *p_portC &= ~0x10;
        led_m = 0;
    }
    if (led == 'b') {
        *p_portC &= ~0x20;
        led_b = 0;
    }
}

void troca_estado_led (char led) {
    if (led == 'c') {
        if (led_c == 1) apaga_led(led);
        else liga_led(led);
    }
    if (led == 'm') {
        if (led_m == 1) apaga_led(led);
        else liga_led(led);
    }
    if (led == 'b') {
        if (led_b == 1) apaga_led(led);
        else liga_led(led);
    }
}
/************** Servicos do LED: END **************/

/************** Servicos de Printar: BEGIN **************/
void comecar_printar(int input) {
    esta_printando = 1; //setta a flag que o programa esta executando um print

    cod_msg = input;
    char *str;
    switch (cod_msg) {
        case 1:
            str = msg_1;
            break;
        case 2:
            str = msg_2;
            break;
        case 3:
            str = msg_3;
            break;
        case 4:
            str = msg_undefined;
            break;
        case 5:
            str = msg_vazio;
            break;
        default:
            break;
    }

    *p_ucsr0B |= 0x40; //habilita a interrupcao TX para printar
    *p_udr0 = str[0]; //coloca o primeiro na saida para desencadear as interrupcoes
    i_msg = 1;
}

void print_msg(char *str) {
    if (!(str[i_msg - 1] == '\n' && str[i_msg] == '\n')) {
        *p_udr0 = str[i_msg];
        i_msg++;
    } else {
        _delay_ms(500);
        *p_ucsr0B &= ~0x40; // apos terminar de enviar a msg, desabilata interrupcao TX
        *p_udr0 = str[i_msg];
        i_msg = 0;
        esta_printando = 0;
    }
}
/************** Servicos de Printar: END **************/


/************** Servicos do Buffer Circular: BEGIN **************/
unsigned int adicionar_indice(unsigned int k) {
    if (k == (MAX_BUFFER-1)) {
        return 0;
    } else {
        return k + 1;
    }
}

void adiciona_buffer(unsigned int in) {
    if (qnt_buffer < MAX_BUFFER) {
        buffer_circular[i] = in;
        qnt_buffer++;
        i = adicionar_indice(i);
    }
}

unsigned char pega_comando_do_buffer() {
    unsigned char output = 0;
    if (qnt_buffer > 0) {
        output = buffer_circular[j];
        buffer_circular[j] = 0;
        qnt_buffer--;
        j = adicionar_indice(j);
    }
    return output;
}
/************** Servicos do Buffer Circular: END **************/

/************** Interrupcoes: BEGIN **************/
/****** RX: ******/
ISR(USART_UDRE_vect) {
        switch (comando_atual) {
            case '1':
                //controle do led 1
                troca_estado_led('c');
                comecar_printar(1);
                break;
            case '2':
                //controle do led 2
                troca_estado_led('m');
                comecar_printar(2);
                break;
            case '3':
                //controle do led 3
                troca_estado_led('b');
                comecar_printar(3);
                break;
            default:
                comecar_printar(4);
                break;
        }
        *p_ucsr0B &= ~0x20; //desabilita interrupcao RX apos fazer acoes necessarias
}

/****** Recebe caracteres ******/
ISR(USART_RX_vect) {
        adiciona_buffer(*p_udr0);
}

/****** TX: envia mensagens ******/
ISR(USART_TX_vect) {
        switch (cod_msg) {
            case 1:
                print_msg(msg_1);
                break;
            case 2:
                print_msg(msg_2);
                break;
            case 3:
                print_msg(msg_3);
                break;
            case 4:
                print_msg(msg_undefined);
                break;
            case 5:
                print_msg(msg_vazio);
                break;
            default:
                break;
        }
}
/************** Interrupcoes: END **************/

int main() {
    config();
    while (1) {
        if (qnt_buffer > 0) {
            //nem interrupcao TX nem DX ligadas
            if ( ((*p_ucsr0B & 0x20) != 0x20)  && (esta_printando == 0) && qnt_buffer != 0) {
                comando_atual = pega_comando_do_buffer();
                *p_ucsr0B |= 0x20; //quando tiver algo no buffer, habilita-se a interrupção RX
            }
        } else {
            //interrupcao DX desligada e nao estar printando no momento
            if ( (((*p_ucsr0B & 0x20) != 0x20) && (esta_printando == 0)) && qnt_buffer == 0) {
                comecar_printar(5);
            }
        }
    }
}