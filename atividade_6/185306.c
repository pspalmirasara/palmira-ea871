/*
 * EA871 - Turma A
 * Palmira Sara Aranovich Florentino RA 185306
 * Atividade 6
 * tinkercard: https://www.tinkercad.com/things/4iS2KJd0pr5-185306-ea871-2s2020-atividade-6/
 */

#define F_CPU 16000000UL
#define MAX_BUFFER 5
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/******** LED ********/
unsigned char *p_portC;
unsigned char *p_pinC;
unsigned char *p_ddrC;
// variaveis que auxiliam a saber se cada LED esta aceso ou apagado
volatile unsigned int led_c = 0; // LED de cima
volatile unsigned int led_m = 0; // LED do meio
volatile unsigned int led_b = 0; // LED de baixo

/******** Estados e Acoes********/
// O programa tera 3 acoes (0, 1 e 2) que serao disparadas pelos comandos 0, 1 e 2
// A variavel acao_atual guardara a acao que esta sendo executada no momento
volatile unsigned int acao_atual = 0;
// Cada acao tera sua propria maquina de estado
volatile unsigned int estado_acao_0 = 0;
volatile unsigned int estado_acao_1 = 0;
volatile unsigned int estado_acao_2 = 0;

/******** Print ********/
volatile unsigned int i_msg = 0;
char msg_0[] = "Comando: Todos os LEDs piscando.\n\n";
char msg_1[] = "Comando: Varredura com um LED acesso.\n\n";
char msg_2[] = "Comando: Varredura com um LED apagado.\n\n";
char msg_undefined[] = "Comando incorreto. \n\n";
char msg_vazio[] = "Vazio! \n\n";
volatile unsigned int esta_printando = 0;
volatile unsigned int cod_msg = 0; //ENUM: 0: msg_0, 1: msg_1, 2: msg_2, 3: msg_undefined, 4: msg_vazio

/******** Buffer Circular ********/
volatile unsigned int i = 0; // Indica a posicao em que devera ser inserido
volatile unsigned int j = 0; // Indica a posicao em que devera ser deletado
volatile unsigned int qnt_buffer = 0; // Quantidade de itens no buffer
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
    //PORTC5: LED de baixo, PORTC4: LED do meio, PORTC3: LED de cima
    *p_portC = 0; // Todos os LEDs começam desligados
    *p_ddrC |= 0x38; // Configura os LEDs como saida

    /*Enderecos do UART*/
    p_ubrr0H = (unsigned char *) 0xC5;
    p_ubrr0L = (unsigned char *) 0xC4;
    p_ucsr0A = (unsigned char *) 0xC0;
    p_ucsr0B = (unsigned char *) 0xC1;
    p_ucsr0C = (unsigned char *) 0xC2;

    /*Registrador que recebera o dado a ser transmitido para a UART*/
    p_udr0 = (unsigned char *) 0xC6;

    /*Especificacoes da atividade*/

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
    //*inicialmente, a interrupcao de buffer vazio esta desabilitada para o programa nao chama-la logo de inicio
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

    /*Habilita interrupções globais*/
    sei();
}

/************** Servicos do LED: BEGIN **************/
// Funcao que recebe um char ('c', 'm' ou 'b') e liga o LED correspondente
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

// Funcao que recebe um char e apaga o LED correspondente
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

// Se o led correspondente esta apagado, o liga e vice-versa
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


/************** Maquinas de Estado e Acoes dos LEDS: BEGIN **************/
// Acao 0: leds em pisca-pisca
void acao_0 () {
    switch (estado_acao_0) {
        case 0:
            liga_led('c');
            liga_led('m');
            liga_led('b');
            estado_acao_0 = 1;
            break;
        case 1:
            apaga_led('c');
            apaga_led('m');
            apaga_led('b');
            estado_acao_0 = 0;
            break;
        default:
            estado_acao_0 = 0;
            break;
    }
}

// Acao 1: leds em varredura (led ligado varrendo)
void acao_1 () {
    switch (estado_acao_1) {
        case 0:
            liga_led('c');
            apaga_led('m');
            apaga_led('b');
            estado_acao_1 = 1;
            break;
        case 1:
            apaga_led('c');
            liga_led('m');
            apaga_led('b');
            estado_acao_1 = 2;
            break;
        case 2:
            apaga_led('c');
            apaga_led('m');
            liga_led('b');
            estado_acao_1 = 0;
            break;
        default:
            estado_acao_1 = 0;
            break;
    }
}

// Acao 2: leds em varredura (led desligado varrendo)
void acao_2 () {
    switch (estado_acao_2) {
        case 0:
            apaga_led('c');
            liga_led('m');
            liga_led('b');
            estado_acao_2 = 1;
            break;
        case 1:
            liga_led('c');
            apaga_led('m');
            liga_led('b');
            estado_acao_2 = 2;
            break;
        case 2:
            liga_led('c');
            liga_led('m');
            apaga_led('b');
            estado_acao_2 = 0;
            break;
        default:
            estado_acao_2 = 0;
            break;
    }
}

// De acordo com qual acao estiver settada no momento (acao_atual), chamara a funcao para
// que ela realize suas instrucoes e avance para o proximo estado.
// Apos isso, executa o delay
void executa_acao_atual() {
    switch (acao_atual) {
        case 0:
            acao_0();
            break;
        case 1:
            acao_1();
            break;
        case 2:
            acao_2();
            break;
        default:
            break;
    }
    _delay_ms(500);
}
/************** Maquinas de Estado e Acoes dos LEDS: END **************/

/************** Servicos de Printar: BEGIN **************/
void comecar_printar(int input) {
    esta_printando = 1; //setta a flag que o programa esta executando um print

    cod_msg = input;
    char *str;
    switch (cod_msg) {
        case 0:
            str = msg_0;
            break;
        case 1:
            str = msg_1;
            break;
        case 2:
            str = msg_2;
            break;
        case 3:
            str = msg_undefined;
            break;
        case 4:
            str = msg_vazio;
            break;
        default:
            break;
    }

    *p_ucsr0B |= 0x40; //habilita a interrupcao TX para printar
    *p_udr0 = str[0]; //coloca o primeiro caracter na saida para desencadear as interrupcoes
    i_msg = 1;
}

void print_msg(char *str) {
    if (!(str[i_msg - 1] == '\n' && str[i_msg] == '\n')) {
        *p_udr0 = str[i_msg];
        i_msg++;
    } else {
        *p_ucsr0B &= ~0x40; // apos terminar de enviar a msg, desabilata interrupcao TX
        *p_ucsr0A |= 0x40; // setta 1 em TXCn para limpar
        *p_udr0 = str[i_msg];
        i_msg = 0;
        esta_printando = 0;
    }
}
/************** Servicos de Printar: END **************/


/************** Servicos do Buffer Circular: BEGIN **************/
void adiciona_buffer(unsigned char in) {
    if (qnt_buffer < MAX_BUFFER) {
        buffer_circular[i] = in;
        qnt_buffer++;
        if ( i == (MAX_BUFFER-1)) i = 0;
        else i++;
    }
}

unsigned char pega_comando_do_buffer() {
    unsigned char out = 0;
    if (qnt_buffer > 0) {
        out = buffer_circular[j];
        qnt_buffer--;
        if ( j == (MAX_BUFFER-1)) j = 0;
        else j++;
    }
    return out;
}
/************** Servicos do Buffer Circular: END **************/

/************** Interrupcoes: BEGIN **************/
/****** UDRE:  ******/
ISR(USART_UDRE_vect) {
        switch (comando_atual) {
            case '0':
                acao_atual = 0;
                comecar_printar(0);
                break;
            case '1':
                acao_atual = 1;
                comecar_printar(1);
                break;
            case '2':
                acao_atual = 2;
                comecar_printar(2);
                break;
            default:
                comecar_printar(3);
                break;
        }
        *p_ucsr0B &= ~0x20; //desabilita interrupcao UDRE apos fazer as acoes necessarias
}

/****** RX: recebe os caracteres do monitor serial ******/
// A interrupcao RX eh disparada sempre que ha um novo caracter na entrada do monitor serial
// O valor desse caracter esta em *p_udr0, que entao eh adicionado ao buffer circular
ISR(USART_RX_vect) {
        adiciona_buffer(*p_udr0);
}

/****** TX: envia mensagens ao monitor serial ******/
// A interrupcao TX sera utilizada para enviar caracteres ao monitor serial (printar)
ISR(USART_TX_vect) {
        switch (cod_msg) {
            case 0:
                print_msg(msg_0);
                break;
            case 1:
                print_msg(msg_1);
                break;
            case 2:
                print_msg(msg_2);
                break;
            case 3:
                print_msg(msg_undefined);
                break;
            case 4:
                print_msg(msg_vazio);
                break;
            default:
                break;
        }
}
/************** Interrupcoes: END **************/

int main() {
    config();

    //coloca o primeiro caracter (da msg vazia) na saida para desencadear as interrupcoes (a TX, especificamente)
    *p_udr0 = msg_vazio[0];
    i_msg = 1;

    while (1) {
        if (qnt_buffer > 0) {
            // Caso haja algo no buffer circular, executara a proxima acao que esta nele
            if ( ((*p_ucsr0B & 0x20) != 0x20)  && (esta_printando == 0)) {
                comando_atual = pega_comando_do_buffer();
                *p_ucsr0B |= 0x20; //quando tiver algo no buffer circular, habilita-se a interrupção UDRE
                executa_acao_atual();
            }
        } else {
            // Caso o buffer circular esteja vazio, imprimira a msg "Vazio!"
            if ( (((*p_ucsr0B & 0x20) != 0x20) && (esta_printando == 0))) {
                esta_printando = 1; //setta a flag que o programa esta executando um print
                cod_msg = 4;
                *p_ucsr0B |= 0x40; //habilita a interrupcao TX para printar
                executa_acao_atual();
            }
        }
    }
}