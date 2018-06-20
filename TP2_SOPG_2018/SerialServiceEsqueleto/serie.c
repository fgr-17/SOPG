/**
 * @file serie.c
 *
 * @author Federico G. Roux
 */


/* --------------------------------------- inclusion de archivos -------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <signal.h>

#include <unistd.h>
#include <errno.h>

#include "serie.h"
#include "SerialManager.h"
#include "manejoThreads.h"

/* --------------------------------------- funciones -------------------------------------- */

int abrirPuertoSerie (void);
void* threadAtenderCIAA (void* p);

/* --------------------------------------- variables globales -------------------------------------- */

/* --------------------------------------- funciones -------------------------------------- */

/**
 * @fn int abrirPuertoSerie (void)
 *
 * @brief abro tty del puerto serie de la CIAA
 * 
 */

int abrirPuertoSerie (void) {

    char mensaje [MENSAJE_L];
     /* abro puerto serie */
    printf("Abriendo puerto serie...");
    if(serial_open(TTY, BAUDRATE)) {
        sprintf(mensaje, "error al abrir puerto. Esta conectada la CIAA? %s\nSaliendo...", TTY_TEXTO);
        perror(mensaje);
        return 1;        
    }
    printf("Puerto serie abierto: %s %d 8N1\n", TTY_TEXTO, BAUDRATE);

    return 0;
}


/**
 * @fn int lanzarThreadAtenderCIAA (pthread*pAtenderCIAA)
 *
 * @brief lanzo el thread que se comunica con la CIAA por el pto. serie
 * 
 */

int lanzarThreadAtenderCIAA (pthread_t*pAtenderCIAA) {

    printf("Generando thread para iniciar atencion de CIAA a traves de puerto serie\n");
    if(bloquearSign()) {
        perror("lanzarThreadAtenderCIAA - error de bloquearSign()");
        return 1;
    }

    if(pthread_create (pAtenderCIAA, NULL, threadAtenderCIAA, NULL)) {
        perror("lanzarThreadAtenderCIAA - pthread_create()");
        return 1;
    }

    if(desbloquearSign()) {
        perror("lanzarThreadAtenderCIAA - error de desbloquearSign()");
        return 1;
    }

    return 0;
}

/**
 * @fn void* threadAtenderCIAA (void* p)
 *
 * @brief abro tty del puerto serie de la CIAA
 * 
 */

void* threadAtenderCIAA (void* p) {

    char serial_rec_buf [SERIAL_REC_BUF_L];
    int bytes_recibidos;

    
    while(1) {
        bytes_recibidos  = serial_receive(serial_rec_buf, SERIAL_REC_BUF_L);
        
        if(bytes_recibidos < 0) {
            perror("serial_receive");
        }
        else if(bytes_recibidos > 0) {

            printf("n: %d %s",bytes_recibidos, serial_rec_buf);

            
        
        }
        usleep(100000);
    }

    return 0;
}

/**
 * @fn void* threadAtenderCIAA (void* p)
 *
 * @brief abro tty del puerto serie de la CIAA
 * 
 */


