#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* Letras minusculas y mayusculas.
El programa determina el numero de letras minusculas y mayusculas que hay en el archivo. */

void minymay(FILE *);

void main(void)
{
  FILE *ar;
  if ((ar = fopen("arc5.txt", "r")) != NULL)
  {
      minymay(ar);
      fclose(ar);
  }
  else
      printf("No se pudo abrir el archivo");
}

void minymay(FILE *arc)
{
  int min = 0, may = 0;
  char p;

  while (!feof(arc))
  {
      p = fgetc(arc);
      if (islower(p))
          min++;
      else
          if (isupper(p))
              may++;
  }
  printf("\nNumero de minusculas: %d", min);
  printf("\nNumero de mayusculas: %d", may);
}