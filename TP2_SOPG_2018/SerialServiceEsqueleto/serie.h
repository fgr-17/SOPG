/**
 * @file serie.h
 *
 * @author Federico G. Roux
 */


/* --------------------------------------- definicion de ctes. -------------------------------------- */

#define BAUDRATE                115200
#define TTY                     1
#define TTY_TEXTO               "/dev/ttyUSB1"
#define SERIAL_REC_BUF_L        128

#define MENSAJE_L               64
#define BUF_DATOS               128
/* --------------------------------------- funciones externas -------------------------------------- */

extern int abrirPuertoSerie (void);
extern int cerrarPuertoSerie (void);

extern int lanzarThreadAtenderCIAA (pthread_t*pAtenderCIAA);
extern int finalizarThreadAtenderCIAA (pthread_t atenderCIAA_thread);
