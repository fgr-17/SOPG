/* ------------------------ definicion de constantes ------------------------------ */

#define FIFO_NAME "myfifo"


#define ERROR_MKNOD             -1
#define ERROR_OPEN_FIFO         -2
#define ERROR_FGETS             -3
#define ERROR_READER_CERRADO    -4
#define ERROR_ESCRIBIR_SIGUSR1  -5
#define ERROR_ESCRIBIR_SIGUSR2  -6

#define CADENA_L                300

#define MSJ_SALIDA_SIGPIPE      "ERROR: se cerro el programa <<reader>>. Saliendo.. \n"
#define MSJ_TERMINAL_SIGUSR1    "Se recibio SIGUSR1. Envio a reader\n"
#define MSJ_TERMINAL_SIGUSR2    "Se recibio SIGUSR2. Envio a reader\n"

#define PREFIJO_TEXTO           "DATA:"
#define MENSAJE_SIGUSR1         "SIGN:1"
#define MENSAJE_SIGUSR2         "SIGN:2"