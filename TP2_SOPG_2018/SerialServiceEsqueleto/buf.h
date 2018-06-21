/**
 * @file buf.h
 *
 * @author Federico G. Roux
 */

/* --------------------------------------- definicion de ctes. -------------------------------------- */
#define BUF_DATOS               128


/* --------------------------------------- tipos de datos -------------------------------------- */
typedef enum {DATOS_NO_LEIDOS, DATOS_LEIDOS, DATOS_INACTIVO} estadoBufDatos_t;

typedef struct {

    pthread_mutex_t mutexBuf;
    estadoBufDatos_t estadoBufDatos;
    char bufDatos [BUF_DATOS];
} buf_t;

/* --------------------------------------- variables globales -------------------------------------- */

extern int inicializarBuf (buf_t *buf);
extern int escribirBuf (buf_t * buf, char* cadena, int len);
extern int leerBuf (buf_t * buf, char* cadena, int len);

