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

#include <signal.h>

#include "reader.h"
#include "cadenastipo.h"
/* ------------------------ funciones ------------------------------ */

/**
 * @fn void sigpipe_handler(int sig)
 *
 * @brief capturo SIGPIPE para el caso que si se cerro el reader y quiera escribir en la fifo
 */

void sigpipe_handler(int sig)
{
   // write(0, MSJ_SALIDA_SIGPIPE, sizeof(MSJ_SALIDA_SIGPIPE));
   // close(fd);
   // exit(ERROR_READER_CERRADO);
exit(0);
}


/**
 * @fn void main (void)
 *
 * @brief leo del programa <<writer>> y guardo en archivos
 */

int main(void)
{
	char cadena[CADENA_L];
    char cadenaBis [CADENA_L];
	int num, fd;
    FILE* fdSign, *fdLog;
    struct sigaction sa;


    printf("Sistemas Operativos de Proposito General\n");
    printf("Trabajo Practico Nª1 - >> reader <<\n\n");


    /* ------ instalo sigpipe ------ */
    sa.sa_handler = sigpipe_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("Error al instalar handler de SIGPIPE");
        exit(1);
    }
    printf("\tHandler de SIGPIPE instalado\n");


    /* ------ instalo fifo ------ */
    printf("Accediendo a FIFO : %s ...\n", FIFO_NAME);
    if(mknod(FIFO_NAME, S_IFIFO | 0666, 0) != 0) {
        if(errno == EEXIST)
            printf("\t> ya existe el archivo <<myfifo>>\n\n");
        else {
            perror("\t> Error al crear la FIFO");
            return ERROR_MKNOD;
        }
    }
    else 
        printf("\t> Archivo <<myfifo>> creado por reader\n\n");

    /* ------ abro fifo y conecto writer ------ */
	printf("\nEsperando ejecución del programa <<writer>>...\n");
	fd = open(FIFO_NAME, O_RDONLY);
    if(fd == -1)
    {
        perror("\t> Error al abrir la FIFO. Saliendo...\n");
        return ERROR_OPEN_FIFO;
    }
	printf("\t> Programa <<writer>> conectado. \n");


    /* ------ abro archivo sign ------ */
    fprintf(stdout, "\nAbriendo archivo %s\n..", ARCHIVO_SIGN);
	fdSign = fopen(ARCHIVO_SIGN, "a+w");
    if(fdSign == NULL)
    {
        perror("\t> Error al abrir archivo de señales. Saliendo...\n");
        close(fd);  
        return ERROR_OPEN_SIGN;
    }
    printf("\t> Archivo %s creado\n", ARCHIVO_SIGN);


    /* ------ abro archivo log ------ */
    fprintf(stdout, "\nAbriendo archivo %s\n..", ARCHIVO_LOG);
	fdLog = fopen(ARCHIVO_LOG, "a+w");
    if(fdLog == NULL)
    {
        perror("\t> Error al abrir archivo de log. Saliendo...\n");
        
        fclose(fdSign);   
        close(fd); 

        return ERROR_OPEN_LOG;
    }
    printf("\t> Archivo %s creado\n", ARCHIVO_LOG);
    

    /* ------ loop ppal ------ */
    printf("\n\nRecibiendo mensajes desde el programa <<writer>>:\n\n");
	do
	{
		if ((num = read(fd, cadena, CADENA_L)) == -1)
			perror("Error al leer de la FIFO.");
		else if (num > 0)                           // pregunto si no lei EOF
		{


			cadena[num] = '\0';                     // agrego null al final de la cadena

            strcpy(cadenaBis, cadena);
            strtok(cadenaBis, CADENA_DELIM);


            if(!strcmp(cadenaBis, PREFIJO_TEXTO)) {
                printf("\t> Datos recibidos por fifo:");
                fprintf (fdLog, "%s", cadena);
            }
            else if(!strcmp(cadenaBis, PREFIJO_SIGUSRx)) {
                printf("\t> Señal recibida por fifo:");
                fprintf (fdSign, "%s\n", cadena);
            } 
            else {
                printf("\t> Datos sin formato recibidos por fifo:");

            }

            printf("%d bytes leidos: \"%s\"\n", num, cadena);
			
		}
	}

	while (num > 0);

    printf("El programa <<writer>> se cerró inesperadamente. Saliendo..\n");
    /* ------ cierro los archivos antes de salir ------ */
    printf("Cierro archivos de texto y fifo\n");

    fclose(fdLog);
    fclose(fdSign);   
    close(fd); 


  
	return 0;
}
