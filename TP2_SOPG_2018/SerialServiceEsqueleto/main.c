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

#define MSJ_SALIDA_SIGINT       "Recibi SIGINT. Saliendo..."
#define FLAG_SIGINT_INACTIVO    0
#define FLAG_SIGINT_CAPTURADA   1

//tener en cuenta sigint - sigpipe
//enmascarar signals en un solo thread

/* ---------------------------------------variables globales-------------------------------------- */


volatile sig_atomic_t flag_sigint = FLAG_SIGINT_INACTIVO;


/* --------------------------------------- signals -------------------------------------- */

/**
 * @fn void sigint_handler(int sig)
 *
 * @brief capturo SIGINT para el caso que quieran cerrar el programa con Ctrl+C
 */

void sigint_handler(int sig)
{
    write(0, MSJ_SALIDA_SIGINT, sizeof(MSJ_SALIDA_SIGINT));
    flag_sigint = FLAG_SIGINT_CAPTURADA;
//    exit(EXIT_FAILURE);
}

int instalarSIGINT(void) {

    struct sigaction si;

    printf("Instalando handlers de signals...");

    /* ------ instalo sigint ------ */
    //printf("Instalando handler de SIGINT...\n");
    si.sa_handler = sigint_handler;
    si.sa_flags = 0;
    sigemptyset(&si.sa_mask);
    if (sigaction(SIGINT, &si, NULL) == -1) {
        return 1;
    }
    printf("handler de SIGINT instalado\n");


    return 0;
}

/**
 * @fn void sigint_handler(int sig)
 *
 * @brief capturo SIGINT para el caso que quieran cerrar el programa con Ctrl+C
 */

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
    pthread_t servidor_thread;
    pthread_t atenderCIAA_thread;

    pid_t miPID;

    flag_sigint = FLAG_SIGINT_INACTIVO;

    miPID = getpid();           // obtengo el pid de este proceso
    printf("Inicio Serial Service (PID:%d)\r\n", miPID);

    printf("\nIniciando el sistema...\n\n");


    if(instalarSIGINT()) {
        perror("Error al instalar handler de SIGINT");
        return(1);
    }


    if(lanzarThreadAtenderCIAA(&atenderCIAA_thread)) {
        perror ("No se pudo crear thread para iniciar la comunicacion serie\n");
        return 1;
    }

    if(lanzarThreadServidor(&servidor_thread)) {
        perror ("No se pudo crear thread para iniciar el servidor\n");
        return 1;
    }
    
    while(1)
    {
        if(flag_sigint == FLAG_SIGINT_CAPTURADA) {
        
// cerrar threads (cada uno cierra sus archivos)

            if(finalizarThreadAtenderCIAA(atenderCIAA_thread))
                fprintf(stderr, "No se pudo finalizar el thread atenderCIAA");
            if(finalizarThreadServidor(servidor_thread)) 
                fprintf(stderr, "No se pudo finalizar el thread servidor");
            exit(EXIT_SUCCESS);
        }
    }


    exit(EXIT_SUCCESS);
    return 0;
}
