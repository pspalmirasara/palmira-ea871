/* Palmira Sara Aranovich Florentino - RA 185306 */
/* Atividade 01 - EA871 turma A */

/* Contador de bits
 *
 * Este programa conta o numero de bits em um byte
 */
s
#include <stdio.h>

int main() {

  unsigned char entrada;
  unsigned int tmp;
  unsigned int n_bits;

  /* Ler entrada em hexadecimal */
  scanf("%x", &tmp);
  entrada = (unsigned char)tmp;

  /*Em tmp temos o número equivalente a entrada na base 10 (como é uint8, ele tem 8 bits, ou seja, vai de
   0 a 255). Para descobrir o número de bits basta pegar o resto da divisão por dois, e então dividí-lo por 2. 
   Sempre que o resto for 1, é adicionado um bit ao contador*/
  n_bits = 0;
  int resto = 0;
  do {
    resto=tmp%2;
    if(resto==1){
      n_bits++;
    }
    tmp=tmp/2;
  } while(tmp != 0);

  printf("%d\n", n_bits);

  return 0;
}
