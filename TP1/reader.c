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


/* ------------------------ variables globales ------------------------------ */
volatile sig_atomic_t fd;                             // archivo de la fifo


/* ------------------------ funciones ------------------------------ */

/**
 * @fn void sigint_handler(int sig)
 *
 * @brief capturo SIGINT para el caso que quieran cerrar el programa con Ctrl+C
 */

void sigint_handler(int sig)
{
   write(0, MSJ_SALIDA_SIGINT, sizeof(MSJ_SALIDA_SIGINT));
   // close(fd);
   // exit(ERROR_READER_CERRADO);
    /*
    close(fdLog);
    close(fdSign);   
    */
/*
    fclose(fdLog);
    fclose(fdSign);
    close(fd);
    exit(0);
*/
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
    int num;
    struct sigaction sa;

    FILE* fdSign, *fdLog;               // archivos de escritura 

    printf("Sistemas Operativos de Proposito General\n");
    printf("Trabajo Practico Nª1 - >> reader <<\n\n");


    //printf("Para salir del programa <<reader>>, cierre el programa <<writer>>");
    /* ------ instalo sigint ------ */
    printf("Instalando handler de signals...\n");
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0; //SA_RESTART;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error al instalar handler de SIGINT");
        exit(1);
    }
    printf("Handler de SIGINT instalado\n");


    /* ------ instalo fifo ------ */
    printf("\nAccediendo a FIFO : %s ...\n", FIFO_NAME);
    if(mknod(FIFO_NAME, S_IFIFO | 0666, 0) != 0) {
        if(errno == EEXIST)
            printf("Ya existe el archivo <<myfifo>>\n");
        else {
            perror("Error al crear la FIFO");
            return ERROR_MKNOD;
        }
    }
    else 
        printf("Archivo <<myfifo>> creado por <<reader>>\n");


    /* ------ abro fifo y conecto writer ------ */
	printf("\nEsperando ejecución del programa <<writer>>...\n");
	fd = open(FIFO_NAME, O_RDONLY);
    if(fd == -1)
    {
        perror("Error al abrir la FIFO. Saliendo...\n");
        return ERROR_OPEN_FIFO;
    }
	printf("Programa <<writer>> conectado. \n");



    /* ------ abro archivo sign ------ */
    fprintf(stdout, "\nAbriendo archivo %s...\n", ARCHIVO_SIGN);
	fdSign = fopen(ARCHIVO_SIGN, "a+t");
    if(fdSign == NULL)
    {
        perror("Error al abrir archivo de señales. Saliendo...\n");
        close(fd);  
        return ERROR_OPEN_SIGN;
    }
    printf("Archivo %s creado\n", ARCHIVO_SIGN);


    /* ------ abro archivo log ------ */
    fprintf(stdout, "\nAbriendo archivo %s...\n", ARCHIVO_LOG);
	fdLog = fopen(ARCHIVO_LOG, "a+t");
    if(fdLog == NULL)
    {
        perror("Error al abrir archivo de log. Saliendo...\n");
        
        fclose(fdSign);   
        close(fd); 
        
        return ERROR_OPEN_LOG;
    }
    printf("Archivo %s creado\n", ARCHIVO_LOG);
    printf("------------------------------ \n\n\n");

    /* ------ loop ppal ------ */
    printf("Recibiendo mensajes desde el programa <<writer>>:\n\n");
	do
	{
		if ((num = read(fd, cadena, CADENA_L)) == -1) {
			perror("FIFO no leida");
            printf("Cierro archivos de texto y fifo\n");
            fclose(fdLog);
            fclose(fdSign);   
            close(fd); 
            return ERROR_READ_FIFO;


        }
		else if (num > 0)                           // pregunto si no lei EOF
		{


			cadena[num] = '\0';                     // agrego null al final de la cadena

            strcpy(cadenaBis, cadena);
            strtok(cadenaBis, CADENA_DELIM);


            if(!strcmp(cadenaBis, PREFIJO_TEXTO)) {
                printf("Datos recibidos por fifo:");
                fprintf (fdLog, "%s\n", cadena);
            }
            else if(!strcmp(cadenaBis, PREFIJO_SIGUSRx)) {
                printf("Señal recibida por fifo:");
                fprintf (fdSign, "%s\n", cadena);
            } 
            else {
                printf("Datos sin formato recibidos por fifo:");

            }

            printf("%d bytes leidos: \"%s\"\n", num, cadena);
			
		}
        else {
            perror("\nError al leer la fifo");
            printf("Cierro archivos de texto y fifo\n");
            fclose(fdLog);
            fclose(fdSign);   
            close(fd); 
            return ERROR_WRITER_CERRADO;
        }
	}

	while (num > 0);

    printf("Saliendo del programa <<reader>>...\n");
    /* ------ cierro los archivos antes de salir ------ */
    printf("Cierro archivos de texto y fifo\n");

    fclose(fdLog);
    fclose(fdSign);   
    close(fd); 


  
	return 0;
}
