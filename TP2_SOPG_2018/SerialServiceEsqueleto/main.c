/**
 * @file main.c
 */

/* --------------------------------------- inclusion de archivos -------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "server.h"
#include "serie.h"
#include "manejoThreads.h"

/* --------------------------------------- definicion de constantes -------------------------------------- */

#define MSJ_SALIDA_SIGINT   "Recibi SIGINT. Saliendo..."


//tener en cuenta sigint - sigpipe
//enmascarar signals en un solo thread

/* ---------------------------------------variables globales-------------------------------------- */

volatile sig_atomic_t flag_sigint;


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
    pthread_t servidor;
    pthread_t atenderCIAA;

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


    
    if(abrirPuertoSerie()) {
        return 1;
    }


    if(lanzarThreadAtenderCIAA(&atenderCIAA)) {
        perror ("No se pudo crear thread para iniciar la comunicacion serie\n");
        return 1;
    }

    if(lanzarThreadServidor(&servidor)) {
        perror ("No se pudo crear thread para iniciar el servidor\n");
        return 1;
    }
    
    while(1)
    {
       
    }


    exit(EXIT_SUCCESS);
    return 0;
}
