/**
 * @file main.c
 */

/* --------------------------------------- inclusion de archivos -------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>
#include <errno.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>
#include <signal.h>

#include "ClientData.h"
#include "SerialManager.h"

#include "server.h"
/* --------------------------------------- definicion de constantes -------------------------------------- */

#define MSJ_SALIDA_SIGINT   "Recibi SIGINT. Saliendo..."


#define BAUDRATE                115200
#define TTY                     1
#define TTY_TEXTO               "/dev/ttyUSB1"
#define SERIAL_REC_BUF_L        128

//tener en cuenta sigint - sigpipe
//enmascarar signals en un solo thread



/* --------------------------------------- signals -------------------------------------- */

/**
 * @fn void sigint_handler(int sig)
 *
 * @brief capturo SIGINT para el caso que quieran cerrar el programa con Ctrl+C
 */

void sigint_handler(int sig)
{
    write(0, MSJ_SALIDA_SIGINT, sizeof(MSJ_SALIDA_SIGINT));
//    exit(EXIT_FAILURE);
}



/* ---------------------------------------------- programa ppal : ----------------------------------------- */


/**
 * @fn int main(void)
 *
 * @brief prog. ppal. 
 * 
 */

#define MAIN_BLOQUEAR_SIGN_ERROR    1


int main(void)
{
    struct sigaction si;
    char mensaje [MENSAJE_L];
    pthread_t servidor;

    char serial_rec_buf [SERIAL_REC_BUF_L];
    int bytes_recibidos;


    pid_t miPID;

    miPID = getpid();           // obtengo el pid de este proceso
    printf("Inicio Serial Service (PID:%d)\r\n", miPID);

    printf("\nIniciando el sistema...\n\n");

    printf("Instalando handlers de signals...");

    /* ------ instalo sigint ------ */
    //printf("Instalando handler de SIGINT...\n");
    si.sa_handler = sigint_handler;
    si.sa_flags = 0;
    sigemptyset(&si.sa_mask);
    if (sigaction(SIGINT, &si, NULL) == -1) {
        perror("Error al instalar handler de SIGINT");
        exit(1);
    }
    printf("handler de SIGINT instalado\n");

    /* abro puerto serie */
    if(serial_open(TTY, BAUDRATE)) {
        sprintf(mensaje, "error al abrir puerto %s\nSaliendo...", TTY_TEXTO);
        perror(mensaje);
        return 1;        
    }
    printf("Puerto serie abierto: %s %d 8N1\n", TTY_TEXTO, BAUDRATE);

    if(lanzarThreadServidor(&servidor)) {
        perror ("No se pudo crear thread para iniciar las conexiones\n");
        return 1;
    }
    
    while(1)
    {
        bytes_recibidos  = serial_receive(serial_rec_buf, SERIAL_REC_BUF_L);

        if(bytes_recibidos < 0) {
            perror("serial_receive");
        }
        else if(bytes_recibidos > 0) {
            printf("n: %d %s",bytes_recibidos, serial_rec_buf);

            


        
        }
        usleep(100000);
    }






    exit(EXIT_SUCCESS);
    return 0;
}
