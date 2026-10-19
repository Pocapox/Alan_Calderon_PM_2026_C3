#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Cuenta cadenas.
El programa, al recibir dos cadenas de caracteres, calcula e imprime cuantas
veces se encuentra la segunda cadena en la primera. */

void main(void)
{
  char cad1[50], cad2[50], cad0[50];
  char *ptr;
  int i = 0;

  printf("\nIngrese la primera cadena de caracteres: ");
  gets(cad1);

  printf("\nIngrese la cadena a buscar: ");
  gets(cad2);

  strcpy(cad0, cad1);
  ptr = strstr(cad0, cad2);

  while (ptr != NULL)
  {
      i++;
      ptr = strstr(ptr + 1, cad2);
  }

  printf("\nEl numero de veces que aparece la segunda cadena es: %d", i);
}