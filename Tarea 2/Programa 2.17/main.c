#include <stdio.h>
#include <stdlib.h>

/*Nomina.
El programa, al recibir los salarios de 15 profesores, obtine el total dle la
nomina de la universidad
I:variable de tipo entero.
SAL t NOM: variables de tipo real. */

void main (void)
{
    int I;
    float SAL,NOM;
    NOM = 0;
    for (I=1; I<=15; I++)
    {
        printf("\nIngrese el salario del profesor%d:\t",I);
        scanf("%f",&SAL);
        NOM= NOM +SAL;
    }
    printf("\nEl total de la nomina es: %.2f", NOM);
}
