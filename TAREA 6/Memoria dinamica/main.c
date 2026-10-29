#include <stdio.h>
#include <stdlib.h>

int main()
{

    FILE *archivo = fopen("el hueso.txt", "r");
    if (archivo == NULL)
        {
        printf("Error: No se pudo abrir el hueso.txt \n");
        return 1;
    }


    int capacidad = 128;
    int longitud = 0;


    char *texto = (char *)malloc(capacidad * sizeof(char));
    if (texto == NULL) {
        printf("\nError: Fallo en la memoria\n");
        fclose(archivo);
        return 1;
    }

    int c;

    while ((c = fgetc(archivo)) != EOF)
        {
        texto[longitud++] = (char)c;


        if (longitud >= capacidad - 1)
            {
            capacidad *= 2;


            char *temp = (char *)realloc(texto, capacidad * sizeof(char));
            if (temp == NULL) {
                printf("\nError: Fallo la reasignacion de memoria");
                free(texto);
                fclose(archivo);
                return 1;
            }
            texto = temp;
        }
    }

    texto[longitud] = '\0';


    printf(" Contenido del archivo\n%s\n", texto);
    printf("\n Bytes leidos: %d | Memoria total asignada: %d bytes\n", longitud, capacidad);

    free(texto);
    fclose(archivo);

    return 0;
}
