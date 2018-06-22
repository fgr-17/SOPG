/**
 * @file buf.c
 *
 * @author Federico G. Roux
 */


/* --------------------------------------- inclusion de archivos -------------------------------------- */
#include <string.h>
#include <pthread.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "buf.h"

/* --------------------------------------- prototipos -------------------------------------- */

int inicializarBuf (buf_t *buf);
int escribirBuf (buf_t * buf, char* cadena, int len);
int leerBuf (buf_t * buf, char* cadena, int len);

/* --------------------------------------- funciones -------------------------------------- */

/**
 * @fn int inicializarBuf (buf_t * buf)
 *
 * @brief inicializo estructura de datos
 */

int inicializarBuf (buf_t *buf) {

    pthread_mutex_init(&buf->mutexBuf, NULL);
    memset((void*) buf->bufDatos, 0, BUF_DATOS);    
    buf->estadoBufDatos = DATOS_INACTIVO;

    return 0;
}


/**
 * @fn int escribirBuf (buf_t * buf, char* cadena, int len)
 *
 * @brief copio una cadena al buffer y levanto el 
 */

int escribirBuf (buf_t * buf, char* cadena, int len) {

    if(len > BUF_DATOS) {
        // largo mayor a la cadena
        return 1;
    }
       

    if(pthread_mutex_lock (&buf->mutexBuf)) {
        return 1;
    }

    strcpy(buf->bufDatos, cadena);
    buf->estadoBufDatos = DATOS_NO_LEIDOS;

    if(pthread_mutex_unlock (&buf->mutexBuf)) {
        return 1;
    }


    return 0;
}

/**
 * @fn int escribirBuf (buf_t * buf, char* cadena, int len)
 *
 * @brief copio una cadena al buffer y levanto el 
 */

int leerBuf (buf_t * buf, char* cadena, int len) {


    int n;

    if(pthread_mutex_lock (&buf->mutexBuf)) {
        fprintf(stderr, "leerBuf: no pude bloquear el mutex\n");
        return -1;
    }

    if(buf->estadoBufDatos != DATOS_NO_LEIDOS)
    {
        if(pthread_mutex_unlock (&buf->mutexBuf)) {
            fprintf(stderr, "leerBuf: no pude desbloquear el mutex\n");
            return -1;
        }
        // fprintf(stderr, "leerBuf: todos los datos leidos\n");
        return -2;
    }

    if(strlen(buf->bufDatos) > BUF_DATOS) {
        if(pthread_mutex_unlock (&buf->mutexBuf)) {
            fprintf(stderr, "leerBuf: no pude desbloquear el mutex\n");
            return -1;
        }
        // largo mayor a la cadena
        fprintf(stderr, "leerBuf: quiero leer mas datos que el largo total\n"); 
        return -1;
    }
        
    n = strlen(buf->bufDatos);
    strcpy(cadena, buf->bufDatos);
    buf->estadoBufDatos = DATOS_LEIDOS;


    if(pthread_mutex_unlock (&buf->mutexBuf)) {
        fprintf(stderr, "leerBuf: no pude desbloquear el mutex\n");
        return -1;
    }

    return n;
}
