/*
 * EA871 - Turma A
 * Palmira Sara Aranovich Florentino RA 185306
 * tinkercard: https://www.tinkercad.com/things/47QMyy6DJ2Q
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* Ponteiros para os registradores do PORTC */
unsigned char *p_portc;
unsigned char *p_pinc;
unsigned char *p_ddrc;

/* Ponteiros para os registradores do PORTD */
unsigned char *p_portd;
unsigned char *p_pind;
unsigned char *p_ddrd;

/* Ponteiro para o registrador PCICR */
unsigned char *p_pcicr;

/* Ponteiro para o registrador PCMSK1 */
unsigned char *p_pcmsk1;

/* Variavel que guarda a frequencia do motor (em rpm) */
int freq = 0;

/* Variavel que guarda o numero de bordas */
int n_bordas = 0;

/* Funcoes para settar o display, retiradas do lab3 */
void set_0() {
    //0111 111x
    *p_portd |= 0x7E; //settando P1 a P6 com 1 (0111 1110)
    *p_portd &= 0x7F; //settando P7 com 0 (0111 1111)
}

void set_1() {
    //0000 110x
    *p_portd |= 0x0C; //settando os 1 (0000 1100)
    *p_portd &= 0x0D; //settando os 0 (0000 1101)
}

void set_2() {
    //1011 011x
    *p_portd |= 0xB6; //settando os 1 (1011 0110)
    *p_portd &= 0xB7; //settando os 0 (1011 0111)
}

void set_3() {
    //1001 111x
    *p_portd |= 0x9E; //settando os 1 (1001 1110)
    *p_portd &= 0x9F; //settando os 0 (1001 1111)
}

void set_4() {
    //1100 110X
    *p_portd |= 0xCC; //settando os 1 (1100 1100)
    *p_portd &= 0xCD; //settando os 0 (1100 1101)
}

void set_5() {
    //1101 101x
    *p_portd |= 0xDA; //settando os 1 (1101 1010)
    *p_portd &= 0xDB; //settando os 0 (1101 1011)
}

void set_6() {
    //1111 101x
    *p_portd |= 0xFA; //settando os 1 (1111 1010)
    *p_portd &= 0xFB; //settando os 0 (1111 1011)
}

void set_7() {
    //0000 111x
    *p_portd |= 0x0E; //settando os 1 (0000 1110)
    *p_portd &= 0x0F; //settando os 0 (0000 1111)
}

void set_8() {
    //1111 111x
    *p_portd |= 0xFE; //settando os 1 (1111 1110)
    *p_portd &= 0xFF; //settando os 0 (1111 1111)
}

void set_9() {
    //1101 111x
    *p_portd |= 0xDE; //settando os 1 (1101 1110)
    *p_portd &= 0xDF; //settando os 0 (1101 1111)
}

void set_10() {
    //1110 111x
    *p_portd |= 0xEE; //settando os 1 (1110 1110)
    *p_portd &= 0xEF; //settando os 0 (1110 1111)
}

void set_11() {
    //1111 100x
    *p_portd |= 0xF8; //settando os 1 (1111 1000)
    *p_portd &= 0xF9; //settando os 0 (1111 1001)
}

void set_12() {
    //0111 001X
    *p_portd |= 0x72; //settando os 1 (0111 0010)
    *p_portd &= 0x73; //settando os 0 (0111 0011)
}

void set_13() {
    //1011 110x
    *p_portd |= 0xBC; //settando os 1 (1011 1100)
    *p_portd &= 0xBD; //settando os 0 (1011 1101)
}

void set_14() {
    //1111 001x
    *p_portd |= 0xF2; //settando os 1 (1111 0010)
    *p_portd &= 0xF3; //settando os 0 (1111 0011)
}

void set_15() {
    //1110 001x
    *p_portd |= 0xE2; //settando os 1 (1110 0010)
    *p_portd &= 0xE3; //settando os 0 (1110 0011)
}

void set_null() {
    //0000 000x
    *p_portd &= 0; //settando os 0
}

/* Inicializacoes dos perifericos envolvidos no sistema */
void configuracoes_iniciais() {

    cli(); // Desabilitando as interrupções globais

    /* Os enderecos foram retirados do manual do ATmega328P, de maneira a seguir o esquematico
     * disponibilizado */

    /* PORTC: Entrada */
    p_pinc = (unsigned char *) 0x26;
    p_ddrc = (unsigned char *) 0x27;
    p_portc = (unsigned char *) 0x28;

    /* PORTD: Saida */
    p_pind = (unsigned char *) 0x29;
    p_ddrd = (unsigned char *) 0x2A;
    p_portd = (unsigned char *) 0x2B;

    /* Habilitar interrupcoes (explicao detalhada na atribuicao de valores) */
    p_pcicr = (unsigned char *) 0x68;
    p_pcmsk1 = (unsigned char *) 0x6C;

    /****** Settando PC0 (entrada) ******/
    *p_ddrc &= 0xFE; //settando como entrada, PC_DDR0 = 0

    /****** Settando PD (display) apagado ******/
    set_null();

    /****** Settando a interrupção na entrada ******/
    /* os 3 bits menos significativos de PCICR servem para habilitar
     * um conjunto de interrupcoes nas portas PCI, e o grupo que nos interessa
     * eh o grupo PCIE1 */
    // Setta o bit PCIE1: habilita a mascar do grupo associado o PCINT8
    *p_pcicr |= 0x02;

    // Setta o bit PCIE8: habilita a mascara de PCINT8
    /* Como a porta que nos interessa observar a borda de decida e realizar uma
     * interrupcao eh a porta PCINT8 (bit menos significativo da PCMSK1), temos que
     * setta-lo */
    *p_pcmsk1 |= 0x01;

    sei(); // Habilitando as interrupções globais
}

/* Como ja foi habilitado a PCINT1, essa interrupcao sera
 * disparada sempre que houver um pulso em algum dos pinos
 * relacionados ao PCINT1 (PCINT8 eh o que nos interessa */
ISR(PCINT1_vect) {
    int estado_anterior = *p_pinc & 0x01; //PC0 eh o bit menos significativo do PC
        if (estado_anterior == 0) { //caso seja detectado uma borda de descida
            n_bordas = n_bordas + 1;
        }
}

/* De acordo com a entrada, setta o display desse numero
 * Caso nao seja nenhum numero entre 0 e 15, apaga o display */
void set_display(int n) {
    switch (n) {
        case 0:
            set_0();
            break;
        case 1:
            set_1();
            break;
        case 2:
            set_2();
            break;
        case 3:
            set_3();
            break;
        case 4:
            set_4();
            break;
        case 5:
            set_5();
            break;
        case 6:
            set_6();
            break;
        case 7:
            set_7();
            break;
        case 8:
            set_8();
            break;
        case 9:
            set_9();
            break;
        case 10:
            set_10();
            break;
        case 11:
            set_11();
            break;
        case 12:
            set_12();
            break;
        case 13:
            set_13();
            break;
        case 14:
            set_14();
            break;
        case 15:
            set_15();
            break;
        default:
            set_null();
            break;
    }
}

/* Aplica as regras descritas no roteiro
 * de qual numero deve aparecer no display
 * de acordo com a frequencia do motor (entrada) */
void calc_numero_display(int f) {
    if (f <= 999) {
        set_display(f / 100);
    } else {
        set_display(9);
    }
}

/* Aplica as regras descritas no roteiro
 * de qual numero deve aparecer no display
 * de acordo com a frequencia do motor (entrada) */
void calcula_frequencia() {
    int bordas_por_volta = 49;
    int tempo = 1;
    int denominador = bordas_por_volta * tempo;
    float frequencia = n_bordas / denominador;
    freq = (int) frequencia;
    calc_numero_display(freq);
}

int main(void) {
    configuracoes_iniciais();

    /* Loop infinito */
    while (1) {
        _delay_ms(60000); //1 minuto
        calcula_frequencia();
        n_bordas = 0; // Eh preciso ficar zerando o numero de bordas para conta-las a partir
        //do inicio do tempo de 1 minuto
    };

    return 0;
}