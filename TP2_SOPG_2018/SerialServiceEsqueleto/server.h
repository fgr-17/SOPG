/* --------------------------------------- definicion de ctes -------------------------------------- */

#define BACKLOG                 100
#define BUFFER_CLIENTE_MAX      128

#define IP                      "127.0.0.1"
#define PORT                    (10000)

#define MENSAJE_L               100




/* --------------------------------------- variables globales -------------------------------------- */
extern int cerrarClientes(void);

extern int lanzarThreadServidor (pthread_t*servidor);
extern int lanzarThreadCliente (int newfd);

extern void* threadServidor (void* p);
extern void* threadAtenderCliente (void* pConexion);


extern pthread_mutex_t mutexBufferCliente;
extern char bufferCliente [BUFFER_CLIENTE_MAX];
