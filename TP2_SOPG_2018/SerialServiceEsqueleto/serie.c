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


#include "buf.h"
#include "serie.h"
#include "SerialManager.h"
#include "manejoThreads.h"


/* --------------------------------------- funciones -------------------------------------- */

int abrirPuertoSerie (void);
void* threadAtenderCIAA (void* p);
int finalizarThreadAtenderCIAA (pthread_t atenderCIAA_thread);

static int analizarPaqueteCIAA (char* cadena);
static int extraerIDCIAA (char* cadena);

/* --------------------------------------- variables globales -------------------------------------- */

buf_t bufSerieRx;
buf_t bufSerieTx;

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
 * @fn int cerrarPuertoSerie (void)
 *
 * @brief cierro tty del puerto serie de la CIAA
 * 
 */

int cerrarPuertoSerie (void) {
     /* cierro puerto serie */
    printf("Cerrando puerto serie...");
    serial_close();
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


    if(inicializarBuf(&bufSerieRx)) {
        fprintf(stderr, "Error al inicializar estructura de datos del buffer\n");
        return 1;
    }

    if(inicializarBuf(&bufSerieTx)) {
        fprintf(stderr, "Error al inicializar estructura de datos del buffer\n");
        return 1;
    }


    if(abrirPuertoSerie()) {
        printf("No se pudo abrir el puerto serie\n");
        return 1;
    }


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
 * @fn int lanzarThreadAtenderCIAA (pthread*pAtenderCIAA)
 *
 * @brief lanzo el thread que se comunica con la CIAA por el pto. serie
 * 
 */

int finalizarThreadAtenderCIAA (pthread_t atenderCIAA_thread) {

    printf("\nFinalizando thread para de atencion de CIAA a traves de puerto serie\n");

    if(cerrarPuertoSerie()) {
        printf("No se pudo cerrar el puerto serie\n");
        return 1;
    }

    if(pthread_cancel (atenderCIAA_thread)) {
        perror("finalizarThreadAtenderCIAA - pthread_cancel()");
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

    char serial_rx_buf [SERIAL_BUF_L];
    char serial_tx_buf [SERIAL_BUF_L]; 
    int bytes_recibidos;
    int bytes_enviar;


    fprintf(stderr, "atiendo cliente\n");    
    while(1) {
        

        bytes_recibidos  = serial_receive(serial_rx_buf, SERIAL_BUF_L);
        

        if(bytes_recibidos < 0) {
            fprintf(stderr, "error al leer puerto serie\n");
        }
        else if(bytes_recibidos > 0) {

            // printf("SERIE: n: %d - %s",bytes_recibidos, serial_rx_buf);

            switch(analizarPaqueteCIAA(serial_rx_buf)) {
            
            case CADENA_OK:
                // no hago nada
                printf("SERIE: La CIAA envio OK\n");
                break;

            case CADENA_ID:
                printf("SERIE: La CIAA envio ID (%d)\n", extraerIDCIAA(serial_rx_buf));
                if(escribirBuf(&bufSerieRx, serial_rx_buf, bytes_recibidos)) {
                    fprintf(stderr, "No se pudo escribir el buffer de recepcion del puerto Serie\n");
                }                
                break;
            default:
                // no hago nada si es otra cosa                
                printf("SERIE: La CIAA envio cadena con error\n");
            }

        }

        if((bytes_enviar = leerBuf(&bufSerieTx, serial_tx_buf, SERIAL_BUF_L)) > 0)
        {
           // fprintf(stderr, "serial send\n");
            // tengo datos para enviar
            serial_send(serial_tx_buf, bytes_enviar);
        }

    
        usleep(10000);
    }

    return 0;
}

/**
 * @fn int analizarPaqueteCIAA (char* cadena)
 *
 * @brief analizo el paquete que me envio la CIAA
 * 
 */

static int analizarPaqueteCIAA (char* cadena) {

    char cad[16];

    strcpy(cad, cadena);
    cad[CAR_FINAL] = '\0';
        

    if(strcmp(cad, ">OK") == 0) {
        return CADENA_OK;
    }
    else if (strcmp(cad, ">ID") == 0) {
        return CADENA_ID;
    }
    else {
        return CADENA_ERROR;
    }    
    
    return 0;
}

/**
 * @fn int extraerIDCIAA (char* cadena)
 *
 * @brief analizo el paquete que me envio la CIAA
 * 
 */

static int extraerIDCIAA (char* cadena) {

    char cad[16];
    char ID_char [5];
    char*ptok;

    int ID_int;

    strcpy(cad, cadena);
    

    ptok = strtok(cad, ":");
    ptok = strtok(NULL, "\r");
    strcpy(ID_char, ptok);

    ID_int = atoi(ID_char);
    
    return ID_int;
}



