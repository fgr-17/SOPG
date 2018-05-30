/**
 * @file reader.c
 * 
 * @author Roux, Federico G. 
 */

/* ------------------------ inclusion de archivos ------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "reader.h"

/* ------------------------ funciones ------------------------------ */

/**
 * @fn void main (void)
 *
 * @brief leo del programa <<writer>> y guardo en archivos
 */

int main(void)
{
	char cadena[CADENA_L];
	int num, fd;
    FILE* fdSign, *fdLog;

    printf("Sistemas Operativos de Proposito General\n");
    printf("Trabajo Practico Nª1 - >> reader <<\n\n");


    printf("Accediendo a FIFO : %s ...\n", FIFO_NAME);
    if(mknod(FIFO_NAME, S_IFIFO | 0666, 0) != 0) {
    

        if(errno == EEXIST)
            printf("ya existe el archivo <<myfifo>>\n\n");
        else {
            perror("Error al crear la FIFO");
            return ERROR_MKNOD;
        }
    }
    else 
        printf("archivo <<myfifo>> creado por reader\n\n");


	printf("Esperando ejecución del programa <<writer>>...\n");
	fd = open(FIFO_NAME, O_RDONLY);
    if(fd == -1)
    {
        perror("Error al abrir la FIFO. Saliendo...\n");
        return ERROR_OPEN_FIFO;
    }
	printf("Programa <<writer>> conectado. \n");



	fdSign = fopen(ARCHIVO_SIGN, "a+w");
    if(fdSign == NULL)
    {
        perror("Error al abrir archivo de señales. Saliendo...\n");
        return ERROR_OPEN_SIGN;
    }
    printf("Archivo %s creado\n", ARCHIVO_SIGN);



	fdLog = fopen(ARCHIVO_LOG, "a+w");
    if(fdLog == NULL)
    {
        perror("Error al abrir archivo de log. Saliendo...\n");
        return ERROR_OPEN_LOG;
    }
    printf("Archivo %s creado\n", ARCHIVO_LOG);
    


    printf("\n\nRecibiendo mensajes desde el programa <<writer>>:\n");
	do
	{
		if ((num = read(fd, cadena, CADENA_L)) == -1)
			perror("Error al leer de la FIFO.");
		else if (num > 0)                           // pregunto si no lei EOF
		{
			cadena[num] = '\0';
			printf(": se leyeron %d bytes: %s\n", num, cadena);
		}
	}
	while (num > 0);

    printf("EL programa <<writer>> se cerró inesperadamente. Saliendo..\n");
  
	return 0;
}
