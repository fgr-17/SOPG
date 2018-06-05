/* ------------------------ definicion de constantes ------------------------------ */

#define FIFO_NAME "myfifo"


#define ERROR_MKNOD             -1
#define ERROR_OPEN_FIFO         -2
#define ERROR_FGETS             -3
#define ERROR_READER_CERRADO    -4
#define ERROR_ESCRIBIR_SIGUSR1  -5
#define ERROR_ESCRIBIR_SIGUSR2  -6
#define ERROR_ESCRIBIR_FIFO     -7
#define ERROR_SIGINT            -8

#define MSJ_SALIDA_SIGPIPE      "\nSe cerro el programa <<reader>>. Saliendo... \n"
#define MSJ_SALIDA_SIGINT       "\nSe cerro el programa <<writer>>. Saliendo... \n"
#define MSJ_TERMINAL_SIGUSR1    "\nSe recibio SIGUSR1. Envio a reader\n"
#define MSJ_TERMINAL_SIGUSR2    "\nSe recibio SIGUSR2. Envio a reader\n"
#define MSJ_INGRESE_TEXTO       "Ingrese texto o envie SIGUSR1 o SIGUSR2 para enviar a reader: "
