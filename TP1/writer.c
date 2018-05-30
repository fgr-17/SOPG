/**
 * @file writer.c
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
#include <sys/wait.h>
#include <unistd.h>

#include <signal.h>

#include "writer.h"

/* ------------------------ variables globales ------------------------------ */
int fd;

/* ------------------------ funciones ------------------------------ */


/**
 * @fn void sigpipe_handler(int sig)
 *
 * @brief capturo SIGPIPE para el caso que si se cerro el reader y quiera escribir en la fifo
 */

void sigpipe_handler(int sig)
{
    write(0, MSJ_SALIDA_SIGPIPE, sizeof(MSJ_SALIDA_SIGPIPE));
    close(fd);
    exit(ERROR_READER_CERRADO);
}

/**
 * @fn void sigusr1_handler(int sig)
 *
 * @brief capturo SIGUSR1 para enviar a reader
 */

void sigusr1_handler(int sig)
{
    int num;
    
    if ((num = write(fd, MENSAJE_SIGUSR1, strlen(MENSAJE_SIGUSR1)+1)) == -1) {
			perror("Error al escribir en la FIFO.");
            exit(ERROR_ESCRIBIR_SIGUSR1);
    }
	else {
        write(0, MSJ_TERMINAL_SIGUSR1, sizeof(MSJ_TERMINAL_SIGUSR1));    
    }
    return;
}

/**
 * @fn void sigusr2_handler(int sig)
 *
 * @brief capturo SIGUSR2 para enviar a reader
 */

void sigusr2_handler(int sig)
{
    int num;
    
    if ((num = write(fd, MENSAJE_SIGUSR2, strlen(MENSAJE_SIGUSR2)+1)) == -1) {
			perror("Error al escribir en la FIFO.");
            exit(ERROR_ESCRIBIR_SIGUSR2);
    }
	else {
        write(0, MSJ_TERMINAL_SIGUSR2, sizeof(MSJ_TERMINAL_SIGUSR2));    
    }
    return;
}


/**
 * @fn void main (void)
 *
 * @brief escribo en una fifo al programa <<reader>>
 */


int main(void)
{
	char cadena[CADENA_L];
    char*cadenaBis;
	int num;
    
    struct sigaction sa;
    struct sigaction sa_sigusr1;
    struct sigaction sa_sigusr2;

    printf("Sistemas Operativos de Proposito General\n");
    printf("Trabajo Practico Nª1 - >> writer <<\n\n");


    printf("Instalando handlers de señales\n");


    /* ------ instalo sigpipe ------ */
    sa.sa_handler = sigpipe_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("Error al instalar handler de SIGPIPE");
        exit(1);
    }
    printf("\tHandler de SIGPIPE instalado\n");

    /* ------ instalo sigusr1 ------ */
    sa_sigusr1.sa_handler = sigusr1_handler;
    sa_sigusr1.sa_flags = SA_RESTART;
    sigemptyset(&sa_sigusr1.sa_mask);
    if (sigaction(SIGUSR1, &sa_sigusr1, NULL) == -1) {
        perror("Error al instalar handler de SIGUSR1");
        exit(1);
    }
    printf("\tHandler de SIGUSR1 instalado\n");

    /* ------ instalo sigusr2 ------ */
    sa_sigusr2.sa_handler = sigusr2_handler;
    sa_sigusr2.sa_flags = SA_RESTART;
    sigemptyset(&sa_sigusr2.sa_mask);

    if (sigaction(SIGUSR2, &sa_sigusr2, NULL) == -1) {
        perror("Error al instalar handler de SIGUSR2");
        exit(1);
    }
    printf("\tHandler de SIGUSR2 instalado\n");

    /* ------ creo fifo ------ */
    printf("\nAccediendo a FIFO : %s ...\n", FIFO_NAME);
    if(mknod(FIFO_NAME, S_IFIFO | 0666, 0) != 0) {
    

        if(errno == EEXIST)
            printf("\tYa existe un archivo <<myfifo>>\n\n");
        else {
            perror("Error al crear la FIFO");
            return ERROR_MKNOD;
        }
    }
    else 
        printf("archivo <<myfifo>> creado por writer\n\n");

    /* ------ abro fifo ------ */
	printf("Esperando ejecución del programa <<reader>>...\n");
	fd = open(FIFO_NAME, O_WRONLY);

    if(fd == -1)
    {
        perror("Error al abrir la FIFO. Saliendo...\n");
        return ERROR_OPEN_FIFO;
    }



    /* ------ loop ppal: ------ */


	printf("Programa <<reader>> conectado. \n");
	while (1)
	{
        printf("Ingrese texto para enviar a reader:");
		cadenaBis = fgets(cadena, CADENA_L,stdin);

        if(cadenaBis != cadena) {
            perror("Error al leer del buffer stdin. Saliendo...\n");
            return ERROR_FGETS;
        }

		if ((num = write(fd, cadena, strlen(cadena))) == -1) {
			perror("Error al escribir en la FIFO.");
            return -1;
            
        }
		else
			printf("writer: se escribieron %d bytes\n", num);
	}
	return 0;
}