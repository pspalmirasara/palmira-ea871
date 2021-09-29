#include <stdio.h>

// Trecho do discurso do presidente da assembléia constituinte,
// Ulysses Guimarães, proferido por ocasião da promulgação da Constituição de 1988.
char texto[] = "A Constituição certamente não é perfeita. Ela própria o confessa ao admitir a reforma. Quanto a ela, discordar, sim. Divergir, sim. Descumprir, jamais. Afrontá-la, nunca.\nTraidor da Constituição é traidor da Pátria. Conhecemos o caminho maldito. Rasgar a Constituição, trancar as portas do Parlamento, garrotear a liberdade, mandar os patriotas para a cadeia, o exílio e o cemitério.\nQuando após tantos anos de lutas e sacrifícios promulgamos o Estatuto do Homem da Liberdade e da Democracia bradamos por imposição de sua honra.\nTemos ódio à ditadura. Ódio e nojo.\nAmaldiçoamos a tirania aonde quer que ela desgrace homens e nações. Principalmente na América Latina.\n\nUlysses Guimarães, Presidente da Assembleia Nacional Constituinte.\n";

// Verifica se o caracter de entrada representa um fim de palavra
int verifica_fim(char c) {
    if (c == ' ' || c == '.' || c == '-' || c == ',' || c == '\n' || c == '\0') {
        return 1;
    } else {
        return 0;
    }
}

// Recebe a posicao inicial e avanca o ponteiro ate encontrar um caracter que
// nao possa fazer parte de uma palavra, e entao devolve a posicao final dessa
// suposta palavra (se inicio == fim, nao eh uma palavra)
int reconhece_palavra(int inicio, int fim, int* i, char* c) {
    do {
        if (verifica_fim(c[*i]) == 0) fim++;
        *i = *i + 1;
    } while (verifica_fim(c[*i]) == 0);
    return fim;
}


// Chama a funcao "reconhece palavra", e caso seja uma palavra mesmo,
// incrementa o contador de palavras
void reconhece_palavra_e_conta(int inicio, int fim, int* i, char* c, int* count) {
    fim = reconhece_palavra(inicio, fim, i, c);
    if (inicio != fim) {
        *count = *count + 1;
    }
}

// Avanca a posicao do vetor ate nao ser mais um caracter que nao faz parte
// de uma palavra, com excecao do '\0'
// Essa funcao foi feita para casos em que existe mais de um caracter que nao
// faz parte de uma palavra, como por exemplo ponto e virgula seguido de
// espaco: '. '
void escape(int* i, char* c) {
    while(verifica_fim(c[*i]) == 1 && c[*i] != '\0') *i = *i + 1;
}

// A função conta_palavras recebe a string cadeia_de_caracteres como argumento
// e deve retornar o número de palavras presentes nessa string.
int conta_palavras(char * cadeia_de_caracteres) {
    int numero_de_palavras = 0;
    int i = 0;

    // Conta as palavras ate o '\0'
    do {
        escape(&i, cadeia_de_caracteres);
        reconhece_palavra_e_conta(i, i, &i, cadeia_de_caracteres, &numero_de_palavras);
    } while (cadeia_de_caracteres[i] != '\0');

    return numero_de_palavras;
}

// A função imprime_palavras_c deve imprimir todas as
// palavras presentes em cadeia_de_caracteres na sequência
// em que aparecem no texto, uma por linha.
void imprime_palavras_c(char * cadeia_de_caracteres) {
    int i = 0;
    int inicio = 0;
    int fim = 0;

    // Eh chamada a funcao reconhece_palavra, que devolve a posicao do final de cada palavra encontrada
    // Entao, eh verificado se a palavra encontrada comeca com 'c' ou 'C'. Caso comece, a palavra
    // inteira eh printada
    do {
        fim = reconhece_palavra(inicio, fim, &i, cadeia_de_caracteres);
        if (cadeia_de_caracteres[inicio] == 'C' || cadeia_de_caracteres[inicio] == 'c') {
            for (int k = inicio; k < fim; k++) printf ("%c", cadeia_de_caracteres[k]);
            printf("\n");
        }
        escape(&i, cadeia_de_caracteres);
        inicio = i;
        fim = i;
    } while (cadeia_de_caracteres[i] != '\0');

}

int main() {
    printf("O número de palavras no texto é: %d\n", conta_palavras(texto));
    printf("As palavra que começam com a letra c no texto são:\n");
    imprime_palavras_c(texto);
    return 0;
}