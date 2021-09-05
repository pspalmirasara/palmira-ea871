/* ========== Explicacao da utilizacao de mascaras ==========
 *
 * Caso seja feita uma atribuicao direta, como por exemplo '*var = 1', por mais que nossa intencao
 * fosse mudar apenas o bit menos significativo, todos os outros 7 bits serao settados como 0.
 *
 * Para contornar esse problema, utilizaremos:
 *
 * - OR bit a bit: quando queremos SETTAR um bit especifico e PRESERVAR os demais
 *  mascara adequada: possui 0 nos bits que queremos PRESERVAR e 1 nos que queremos SETTAR
 *  exemplo: para settar o apenas o bit menos significativo, utilizariamos a mascara 0x01
 *  modos de escrever: *var = *var | 0x01
 *                     *var |= 0x01
 *
 * - AND bit a bit: quando queremos ZERAR um bit específico e PRESERVAR os demais
 * mascara adequada: possui 1 nos bits que queremos PRESERVAR e 0 nos que queremos ZERAR
 * exemplo: para zerar apenas o bit mais significativo, utilizariamos a mascara 0x7F
 * modos de escrever: *var = *var & 0x7F
 *                    *var &= 0x7F
 * /

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

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

/* Variavel que guarda o estado da MEF */
/* Ela tera os estados de 0 a 15, e depois voltara ao estado 0 */
unsigned char estado = 0;

/* Variavel que guarda o estado do led decimal */
unsigned char led = 0;

/* Mapeamento dos botoes do display*/
/*
 * PD1: a
 * PD2: b
 * PD3: c
 * PD4: d
 * PD5: e
 * PD6: f
 * PD7: g
 * de acordo com o mapeamento encontrado em:
 * https://components101.com/sites/default/files/component_pin/7-segment-display-pin-diagr_0.png
 */

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

void liga_led_decimal() {
    *p_portd |= 0x01;
    led = 1;
}

void apaga_led_decimal() {
    *p_portd &= 0xFE;
    led = 0;
}

/* Inicializacoes dos perifericos envolvidos no sistema */
void configuracoes_iniciais(void) {

    /* MCUCR */
    p_mcucr = (unsigned char *) 0x55;
    /* PORTC: todos os 8 bits estao sendo usados pelo display */
    p_pinc = (unsigned char *) 0x26;
    p_ddrc = (unsigned char *) 0x27;
    p_portc = (unsigned char *) 0x28;
    /* PORTD: o botao esta sendo usado no bit menos significativo */
    p_pind = (unsigned char *) 0x29;
    p_ddrd = (unsigned char *) 0x2A;
    p_portd = (unsigned char *) 0x2B;

    /****** Settando PC0 (botão) ******/
    *p_ddrc &= 0xFE; //settando como entrada, PC_DDR0 = 0

    /*Ativando os resistores de pull-up:
     *O bit 4 do MCUCR deve ser 0 para poderem ser
   *ativados os resistores de pull-up em cada PORT
   */
    *p_mcucr &= 0xEF;
    /*Ativar o pull up na entrada do botao (PORTC menos significativo)*/
    *p_portc |= 0x01;


    /****** Settando PD0 (led decimal) ******/
    /* O bit menos significativo (PD0) indica o led decimal */
    *p_ddrd |= 0x01; //settando PD0 como saida (PD_DDR0 = 1)
    apaga_led_decimal(); //settando o led desligado inicialmente

    /****** Settando PD1 a PD7 (led display) ******/
    /* O display comeca com 0, que e representado por P1, P2, P3, P4, P5, P6 = 1 e P7 = 0*/
    /* Portanto, todas as operacoes para settar numeros nos displays terao que settar apenas os
     * 7 primeiros bits mais significativos, e preservar o estado do bit menos significativo
     * */
    set_0();
}

void maquina_de_estados() {
    switch (estado) {
        case 0:
            if (botao == 1) {
                //repouso
            } else {
                liga_led_decimal();
                set_1();
                estado = 1;
            }
            break;
        case 1:
            if (botao == 1) {
                apaga_led_decimal();
                estado = 2;
            } else {
                //repouso
            }
            break;
        case 2:
            if (botao == 1) {
                //repouso
            } else {
                liga_led_decimal();
                set_2();
                estado = 3;
            }
            break;
        case 3:
            if (botao == 1) {
                apaga_led_decimal();
                estado = 4;
            } else {
                //repouso
            }
            break;
        case 4:
            if (botao == 1) {

            } else {
            }
            break;
        case 5:
            if (botao == 1) {

            } else {
            }
            break;
        case 6:
            if (botao == 1) {

            } else {
            }
            break;
        case 7:
            if (botao == 1) {

            } else {
            }
            break;
        case 8:
            if (botao == 1) {

            } else {
            }
            break;
        default:
            break;
    }
}

void filtragem_transitorio() {
    if (botao != ((*p_pinc & 0x01))) {
        _delay_ms(100);
        if (botao != ((*p_pinc & 0x01))) {
            botao = ((*p_pinc & 0x01));
        }
    }
}

int main(void) {
    configuracoes_iniciais();

    /* Loop infinito */
    while (1) {
        filtragem_transitorio();
        maquina_de_estados();
    }

    return 0;
}