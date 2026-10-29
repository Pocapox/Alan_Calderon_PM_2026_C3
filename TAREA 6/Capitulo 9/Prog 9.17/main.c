#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* Reemplaza palabras.
El programa lee cadenas de caracteres de un archivo y reemplaza la palabra
mexico escrita con minusculas por su forma correcta Mexico. */

void cambia(FILE *, FILE *);

void main(void)
{
  FILE *ar;
  FILE *ap;

  ar = fopen("arc.txt", "r");
  ap = fopen("arc1.txt", "w");

  if ((ar != NULL) && (ap != NULL))
  {
      cambia(ar, ap);
      fclose(ar);
      fclose(ap);
  }
  else
      printf("No se pueden abrir los archivos");
}

void cambia(FILE *ap1, FILE *ap2)
{
  int i, j, k;
  char cad[50], cad1[50], *cad2, aux[50];

  while (!feof(ap1))
  {
      if (fgets(cad, 50, ap1) == NULL)
          break;
      strcpy(cad1, cad);
      cad2 = strstr(cad1, "mexico");
      while (cad2 != NULL)
      {
          cad2[0] = 'M';
          i = strlen(cad1);
          j = strlen(cad2);
          k = i - j;
          if (k)
          {
              strncpy(aux, cad1, k);
              aux[k] = '\0';
              strcat(aux, cad2);
              strcpy(cad1, aux);
          }
          else
              strcpy(cad1, cad2);
          cad2 = strstr(cad1, "mexico");
      }
      fputs(cad1, ap2);
  }
}