#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Cuenta palabras.
El programa calcula el numero de palabras que hay en la cadena de caracteres. */

int cuentap(char *);

void main(void)
{
  int i;
  char fra[50];

  printf("\nIngrese la linea de texto: ");
  gets(fra);

  strcat(fra, " ");
  i = cuentap(fra);
  printf("\nLa linea de texto tiene %d palabras", i);
}

int cuentap(char *cad)
{
  char *cad0;
  int i = 0;

  cad0 = strstr(cad, " ");
  while (cad0 != NULL && strcmp(cad0, " "))
  {
      i++;
      cad0 = strstr(cad0 + 1, " ");
  }

  return (i);
}