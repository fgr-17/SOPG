
/* --------------------------------------- inclusion de archivos -------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>
#include <signal.h>

#include "ClientData.h"


/* --------------------------------------- definicion de constantes -------------------------------------- */

#define BACKLOG          100
#define BUFFER_CLIENTE_MAX      128

#define IP                      "127.0.0.1"
#define PORT                    (4096)

#define MENSAJE_L               100


#define MSJ_BIENVENIDA          "Sistemas Operativos de Proposito General - CESE 2018\n------------------- Servidor TCP -------------------\n"


#define LANZARTHREAD_ERROR_LISTA_LLENA          1
#define LANZARTHREAD_ERROR_PTHREAD_CREATE       2
#define LANZARTHREAD_ERROR_PTHREAD_DETACH       3
#define LANZARTHREAD_ERROR_PTHREAD_MUTEXLOCK    4
#define LANZARTHREAD_ERROR_PTHREAD_MUTEXUNLOCK  5
#define LANZARTHREAD_ERROR_BLOQUEARSIGN         6
#define LANZARTHREAD_ERROR_DESBLOQUEARSIGN      7

#define MSJ_SALIDA_SIGINT   "Recibi SIGINT. Saliendo..."




/* --------------------------------------- prototipos -------------------------------------- */

int cerrarClientes(void);
int lanzarThreadCliente (int newfd);
int bloquearSign(void);
int desbloquearSign(void);
void* atenderCliente (void* pConexion);


/* --------------------------------------- variables globales -------------------------------------- */

ClientData listaConexiones [BACKLOG];
pthread_mutex_t mutexLista = PTHREAD_MUTEX_INITIALIZER;


/* --------------------------------------- funciones -------------------------------------- */

/**
 * @fn int cerrarClientes(void);
 *
 * @brief cierro todos los clientes
 * 
 */

int cerrarClientes(void) {

    int index;

    for (index = 0; index < BACKLOG; index ++) {
        if(!listaConexiones[index].flagFree) {
            
            if(close(listaConexiones[index].fd)) {
                perror("Error de close()");
                exit(EXIT_FAILURE);
            }
            
            if(pthread_cancel(listaConexiones[index].thread)) {
                perror("Error de pthread_cancel()");
                exit(EXIT_FAILURE);
            }
            
        }
    }

    return 0;
}


/**
 * @fn int lanzarThreadCliente (int newfd)
 *
 * @brief rutina que atiende al cliente, genera un lugar en la lista de conexiones y crea un 
 *        thread para escucharlo
 * 
 */

int lanzarThreadCliente (int newfd)
{

    int index;
    int clienteID;

    printf("Accediendo a lista de conexiones...");

    /* intento bloquear el acceso a la lista */
    if(pthread_mutex_lock (&mutexLista)) {
        close(newfd);    
        fprintf(stderr, "error al bloquear el acceso a la lista de conexiones");
        return LANZARTHREAD_ERROR_PTHREAD_MUTEXLOCK;
    }
    index = cd_getFreeIndex(listaConexiones, BACKLOG);


    clienteID = listaConexiones[index].ID;
        
    //printf("cd_getFreeIndex return: %d\n", index);

    if(index == -1) {
        pthread_mutex_unlock (&mutexLista);
        close(newfd);
        fprintf(stderr, "memoria insuficiente para aceptar conexiones\n");
        return LANZARTHREAD_ERROR_LISTA_LLENA;
    }
    printf("lista de conexiones accedida\n");  


    if(bloquearSign()) {
        perror("Error de bloquearSign()");
        return LANZARTHREAD_ERROR_BLOQUEARSIGN;
    }

    printf("Generando thread para atender conexion...");
    if(pthread_create (&listaConexiones[index].thread, NULL, atenderCliente, &listaConexiones[index])) {
        pthread_mutex_unlock (&mutexLista);
        perror("Error de pthread_create()");
        return LANZARTHREAD_ERROR_PTHREAD_CREATE;
    }

    if(pthread_detach(listaConexiones[index].thread)) {
        pthread_mutex_unlock (&mutexLista);
        perror("Error en pthread_detach()");
        return LANZARTHREAD_ERROR_PTHREAD_DETACH;
    }
    printf("hilo de atencion de cliente creado\n");

    listaConexiones[index].flagFree=0;
    listaConexiones[index].fd = newfd;

    if(pthread_mutex_unlock (&mutexLista)) {
        perror("Error al desbloquear el acceso a la lista de conexiones");
        return LANZARTHREAD_ERROR_PTHREAD_MUTEXUNLOCK;
    }
    
    printf("\nSe establecio la conexion %4d\n\n", clienteID);



    if(desbloquearSign()) {
        perror("Error de desbloquearSign()");
        return LANZARTHREAD_ERROR_DESBLOQUEARSIGN;
    }

    // capturar sigint y poner el codigo de cerrar en la salida del accept
    // cerrar todos los threads de la lista que esten flag en cero
    // cerrar todos los sockets de la lista que esten flag en cero
    
    return 0;
}

/**
 * @fn int bloquearSign(void)
 *
 * @brief Bloqueo set de signals que me interesan
 * 
 */

int bloquearSign(void) {

    sigset_t set;

    if(sigemptyset(&set)) {
        perror("Error de sigemptyset()");
        return errno;
    }
        
    if(sigaddset(&set, SIGINT)) {
        perror("Error de sigaddset()");
        return errno;
    }

    if(pthread_sigmask(SIG_BLOCK, &set, NULL)) {
        perror("Error de pthread_sigmask()");
        return errno;
    }

    return 0;
}

/**
 * @fn int desbloquearSign(void)
 *
 * @brief Desbloqueo set de signals que me interesan
 * 
 */

int desbloquearSign(void) {

    sigset_t set;

    if(sigemptyset(&set)) {
        perror("Error de sigemptyset()");
        return errno;
    }
        
    if(sigaddset(&set, SIGINT)) {
        perror("Error de sigaddset()");
        return errno;
    }

    if(pthread_sigmask(SIG_UNBLOCK, &set, NULL)) {
        perror("Error de pthread_sigmask()");
        return errno;
    }

    return 0;
}


/* --------------------------------------- signals -------------------------------------- */

/**
 * @fn void sigint_handler(int sig)
 *
 * @brief capturo SIGINT para el caso que quieran cerrar el programa con Ctrl+C
 */

void sigint_handler(int sig)
{
    write(0, MSJ_SALIDA_SIGINT, sizeof(MSJ_SALIDA_SIGINT));
//    exit(EXIT_FAILURE);
}


/* --------------------------------------- threads -------------------------------------- */


/**
 * @fn void* atenderCliente (void* pConexion)
 *
 * @brief thread que se comunica con el cliente con un socket ya conectado.
 * 
 */

void* atenderCliente (void* pConexion)
{

	int n;
    int clienteID;
	char buffer[BUFFER_CLIENTE_MAX];

    ClientData* pData; 

    int newfd;


//    printf(">>>>Entre a thread<<<<\n");

     /* intento bloquear el acceso a la lista */
    if(pthread_mutex_lock (&mutexLista)) {
        //close(newfd);    
        fprintf(stderr, "Thread: Error al bloquear el acceso a la lista de conexiones");
        return 0;
    }

    pData = (ClientData*) pConexion;
    newfd = pData->fd;
    clienteID = pData->ID;

     /* intento bloquear el acceso a la lista */
    if(pthread_mutex_unlock (&mutexLista)) {
        //close(newfd);    
        fprintf(stderr, "Thread: Error al desbloquear el acceso a la lista de conexiones");
        return 0;
    }



    while(1)
    {

        if( (n =read(newfd,buffer,BUFFER_CLIENTE_MAX)) == -1 )
	    {
		    perror("Thread: Error leyendo mensaje en socket");
		    exit(1);
	    }


        if(n == 0) {

            fprintf(stderr, "Se cerro la conexion del cliente %d\n", clienteID);
            break;      // Se cerro la conexion
        }

	    buffer[n - 1]=0;
	    printf("Recibi %d bytes del cliente %4d :'%s'\n", n, clienteID, buffer);

	    // Enviamos mensaje a cliente
	    if (write (newfd, buffer, strlen(buffer)) == -1)
	    {
      		perror("Thread: Error escribiendo mensaje en socket");
      		exit (1);
	    }
    }
	// Cerramos conexion con cliente
	close(newfd);
    // Levanto el flag.


     /* intento bloquear el acceso a la lista */
    if(pthread_mutex_lock (&mutexLista)) {
        //close(newfd);    
        fprintf(stderr, "Thread: Error al bloquear el acceso a la lista de conexiones");
        return 0;
    }


    /* marco el casillero de cliente como libre */
    pData->flagFree = 1;

     /* intento bloquear el acceso a la lista */
    if(pthread_mutex_unlock (&mutexLista)) {
        //close(newfd);    
        fprintf(stderr, "Thread: Error al desbloquear el acceso a la lista de conexiones");
        return 0;
    }

    return 0;
}


/* ---------------------------------------------- programa ppal : ----------------------------------------- */


/**
 * @fn int main(void)
 *
 * @brief prog. ppal. Creo un socket, escucho conexiones y atiendo con la funcion lanzarThreadCliente
 *        me quedo en el accept mientras en un thread se comunican con los clientes.
 * 
 */


int main(void)
{
    socklen_t addr_len;
    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
    struct sigaction si;

    char mensaje[MENSAJE_L];

    int newfd;

    // Creamos socket
    int s = socket(PF_INET,SOCK_STREAM, 0);

    printf(MSJ_BIENVENIDA);


    printf("\nIniciando el sistema...\n\n");



    printf("Instalando handlers de signals...");

    /* ------ instalo sigint ------ */
    //printf("Instalando handler de SIGINT...\n");
    si.sa_handler = sigint_handler;
    si.sa_flags = 0;
    sigemptyset(&si.sa_mask);
    if (sigaction(SIGINT, &si, NULL) == -1) {
        perror("Error al instalar handler de SIGINT");
        exit(1);
    }
    printf("handler de SIGINT instalado\n");




    // Cargamos datos de IP:PORT del server
    printf("\nGenerando socket para recibir conexion...");
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = inet_addr(IP);
    if(serveraddr.sin_addr.s_addr==INADDR_NONE)
    {
        sprintf(mensaje, "Error al generar el IP %s", IP);
        perror(mensaje);
        return 1;
    }
    printf("se genero la direccion IP correctamente\n");


    // Abrimos puerto con bind()
    printf("\nAbriendo puerto...");    
    if (bind(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
        close(s);
        sprintf(mensaje, "Error al abrir el puerto %d", PORT);
        perror(mensaje);
        return 1;
    }
    printf("el puerto se abrio correctamente\n");

    // Seteamos socket en modo Listening
    printf("\nConfigurando socket en modo <<listen>>...");
    if (listen (s, BACKLOG) == -1) // backlog=BACKLOG
    {
                close(s);
        perror("Error al configurar socket en modo listen");
        exit(1);
    }
    printf("socket configurado en modo <<listen>>\n");


    printf("\nInicializando estructura de datos...");
    cd_init(listaConexiones, BACKLOG);
    printf("estructura de datos inicializada\n");


    printf("-----------------------------------------\n\n\n");
    printf("Esperando nuevas conexiones a %s:%d...\n\n", IP, PORT);
    while(1)
    {
        // Ejecutamos accept() para recibir conexiones entrantes
        addr_len = sizeof(struct sockaddr_in);

        newfd = accept(s, (struct sockaddr *)&clientaddr, &addr_len);
        if ( newfd == -1) {
                   
            if(errno == EINTR) {
                close(s);
                printf("cerrando lista de clientes\n");
                cerrarClientes();
                exit(EXIT_SUCCESS);
            }
            else {
                close(s);
                perror("Error en accept");
                exit(EXIT_FAILURE);
            }

        }
        printf  ("Conexion entrante desde:  %s:%d\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);

        lanzarThreadCliente(newfd);
    }

    return 0;
}

