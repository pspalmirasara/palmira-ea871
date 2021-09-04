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
unsigned char estado = 0;

/* Variavel que guarda o estado da MEF */
unsigned char led = 0;

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
    *p_portd &= 0xFE; //settando o led desligado inicialmente

}

void maquina_de_estados_finitos(void) {
}

void acionamento_led(void) {
    maquina_de_estados_finitos();

    if (led == 1)
        *p_portc |= 1;
    else
        *p_portc &= 0xFE;

}


void filtragem_transitorio(void) {
    if (botao != ((*p_pinc & 0x01))) {
        //_delay_ms(1);
        if (botao != ((*p_pinc & 0x01))) {
            botao = ((*p_pinc & 0x01));
        }
    }
}

void acionamento_led_decimal() {
    int a = 0;
    if ( (*p_pinc & 0x01) == 1) {
        *p_portd &= 0xFE;
    } else {
        *p_portd |= 1;
    }
}

int main(void) {
    configuracoes_iniciais();

    /* Loop infinito */
    while (1) {
        //filtragem_transitorio();
        //acionamento_led();
        acionamento_led_decimal();
    }

    return 0;
}