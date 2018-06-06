/* ------------------------ definicion de constantes ------------------------------ */

#define FIFO_NAME               "myfifo"

#define ARCHIVO_SIGN            "./Sign.txt"
#define ARCHIVO_LOG             "./Log.txt"

#define ERROR_MKNOD             -1
#define ERROR_OPEN_FIFO         -2
#define ERROR_FGETS             -3
#define ERROR_OPEN_SIGN         -4
#define ERROR_OPEN_LOG          -5
#define ERROR_WRITER_CERRADO    -6
#define ERROR_READ_FIFO         -7

#define MSJ_SALIDA_SIGINT       "\nSe recibio SIGINT...\n"
#define MSJ_SALIDA_SIGPIPE      "\nSe recibio SIGPIPE...\n"
