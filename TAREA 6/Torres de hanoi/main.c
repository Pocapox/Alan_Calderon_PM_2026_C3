#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>

void Hanoi (unsigned int n, char origen, char destino, char auxiliar)
{
    if (n == 1)
    {
        printf("Mover disco 1 de la torre %c a la torre %c\n", origen, destino);

    }
    else
    {
        Hanoi(n - 1, origen, auxiliar, destino);
        printf("Mover disco %u de la torre %c a la torre %c\n", n, origen, destino);
        Hanoi(n - 1, auxiliar, destino, origen);
    }
}

int main (void)
{
    unsigned int discos = 3;

    printf("Solucion para %u discos:\n", discos);
    Hanoi(discos, 'A', 'C', 'B');

    return 0;
}
