/*
 * EA871 - Turma A
 * Palmira Sara Aranovich Florentino RA 185306
 * Atividade 7
 * tinkercard: https://www.tinkercad.com/things/jrPw7hy1dBK-185306-atividade-7
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
unsigned char *p_ocr0a;
unsigned char *p_timsk0;
unsigned char *p_tccr0b;
unsigned char *p_tccr0a;

/******** UART ********/
unsigned char *p_ubrr0H;
unsigned char *p_ubrr0L;
unsigned char *p_ucsr0A;
unsigned char *p_ucsr0B;
unsigned char *p_ucsr0C;
unsigned char *p_udr0;

/******** Print ********/
volatile unsigned int i_msg = 0;
char msg[] = "Atividade 7 - Interrupcoes temporizadas tratam concorrencia entre tarefas! \n\n";

/******** Temporizador ********/
volatile unsigned  int contador_1 = 0;
volatile unsigned  int contador_2 = 0;
volatile unsigned char estado = 0;


void config() {
    /*Desabilita interrupções globais*/
    cli();

    /*Enderecos dos temporizadores*/
    p_ocr0a = (unsigned char *) 0x47;
    p_timsk0 = (unsigned char *) 0x6E;
    p_tccr0b = (unsigned char *) 0x45;
    p_tccr0a = (unsigned char *) 0x44;

    /*Endereco da PORTB*/
    p_ddrB = (unsigned char *) 0x24;
    p_portB = (unsigned char *) 0x25;

    /*Enderecos do UART*/
    p_ubrr0H = (unsigned char *) 0xC5;
    p_ubrr0L = (unsigned char *) 0xC4;
    p_ucsr0A = (unsigned char *) 0xC0;
    p_ucsr0B = (unsigned char *) 0xC1;
    p_ucsr0C = (unsigned char *) 0xC2;
    p_udr0 = (unsigned char *) 0xC6;

    /* Especificacoes do temporizador */

    *p_ocr0a = 249; //OCR0A

    // TIMSK0 – Timer/Counter Interrupt Mask Register
    // – – – – – OCIE0B OCIE0A TOIE0
    //   0         0      1      0
    //OCIE0A: contagem resetta quando a comparação é igual ao registrador OCIE0A
    *p_timsk0 = 2;

    // TCCR0B – Timer/Counter Control Register B
    // FOC0A FOC0B – – WGM02 CS02 CS01 CS00
    // 0      0   - -   0    0     1   0
    // CS02, CS01 e CS00: valor do preseter
    *p_tccr0b = 3;

    // TCCR0A – Timer/Counter Control Register A
    // COM0A1 COM0A0 COM0B1 COM0B0 – – WGM01 WGM00
    //  0      0       0     0    - -    1    0
    // WGM02, WGM01 E WGM00: setta o modo (010 para CTC)
    *p_tccr0a = 2;

    /* Especificacoes da UART*/

    // Bound rate: 19,2kbps com a F_CPU definida (16000000UL) e sem double speed (erro de 0.2%)
    // De acordo com o manual, UBRRN0 = 51 (dec) -> 11 0011 (bin) = 0x33 (hex)
    *p_ubrr0H = 0;
    *p_ubrr0L = 0x33;

    // UCSRnA – USART Control and Status Register n A
    // RXCn TXCn UDREn FEn DORn UPEn U2Xn MPCMn
    //  X    X     X    X    X   X    0     0
    // U2Xn: double speed desativado
    // MPCMn: modo de transmissao multi-processador desabilitado
    *p_ucsr0A = 0;

    // UCSRnB – USART Control and Status Register n B
    // RXCIEn TXCIEn UDRIEn RXENn TXENn UCSZn2 RXB8n TXB8n
    //  1       1      0      1     1     0      X     X
    // RXCIEn: habilita interrupcao quando um caractere eh escrito no monitor serial (habilitada)
    // TXCIEn: habilita interrupcao quando uma transmissao termina (transmissao completa) (habilitada)
    // UDRIEn: habilita interrupcao quando o buffer de enviar dados esta vazio (inicialmente, desabilitada*)
    // RXENn e TXENn: habilita recepcao e transmissao (ambas habilitadas)
    // UCSZn2: numero de bits no pacote (1byte - 8bits), entao eh zero
    // RXB8n TXB8n: don't care pq nao estamos usando um 9o bit
    *p_ucsr0B = 0xD8;

    // UCSRnC – USART Control and Status Register n C
    // UMSELn1 UMSELn0 UPMn1 UPMn0 USBSn UCSZn1 UCSZn0 UCPOLn
    //    0       0      0     0     0      1     1      0
    // UMSELn1 UMSELn0: modo de operacao (assincrono)
    // UPMn1 UPMn0: paridade de bits (sem bits de paridade)
    // USBSn: bits de parada (1 bit -> settado para zero)
    // UCSZn1 UCSZn0: quantidade de bits por pacote (1byte - 8bits), pelo manual, ambos habilitados
    // UCPOLn: polaridade do clock, desabilitado no modo assincrono
    *p_ucsr0C = 0x06;

    /* Especificacoes dos LED */
    //The Port B Data Register
    //PORTB5 = pino 13 PORTB4 = pino 12
    // 0000 0011
    // 0011 0000

    *p_ddrB |= 0x30; // Configura os LEDs como saida
    *p_portB &= ~(0x30);  // Todos os LEDs começam desligados

    /*Habilita interrupções globais*/
    sei();
}

/************** Servicos de Printar: BEGIN **************/
void print_msg(char *str) {
    if (!(str[i_msg - 1] == '\n' && str[i_msg] == '\n')) {
        *p_udr0 = str[i_msg];
        i_msg++;
    } else {
        *p_ucsr0B &= ~0x40; // apos terminar de enviar a msg, desabilata interrupcao TX
        *p_ucsr0A |= 0x40; // setta 1 em TXCn para limpar
        *p_udr0 = str[i_msg];
        i_msg = 0;
    }
}
/************** Servicos de Printar: END **************/

/************** Interrupcoes: BEGIN **************/
ISR(TIMER0_COMPA_vect) {
        contador_1++;
        if (contador_1 >= 100) {
            contador_1 = 0;
            if (estado == 0) {
                *p_portB |= (0x10);
                estado = 1;
            } else {
                *p_portB &= ~(0x10);
                estado = 0;
            }
        }

        contador_2++;
        if (contador_2 >= 256) {
            contador_2 = 0;
            if (estado == 0) {
                *p_portB |= (0x20);
                estado = 1;
            } else {
                *p_portB &= ~(0x20);
                estado = 0;
            }
        }
}

ISR(USART_TX_vect) {
    print_msg(msg);
}
/************** Interrupcoes: END **************/

int main() {
    config();

    *p_udr0 = msg[0]; //coloca o primeiro caracter na saida para desencadear as interrupcoes
    i_msg = 1;

    while (1) {
        *p_ucsr0B |= 0x40; //habilita a interrupcao TX para printar
        _delay_ms(5000);
    }
}