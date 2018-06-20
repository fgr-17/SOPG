/**
 * @file server.c
 */

/* --------------------------------------- inclusion de archivos -------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>
#include <errno.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>
#include <signal.h>

#include "ClientData.h"
#include "SerialManager.h"

#include "server.h"

/* --------------------------------------- funciones -------------------------------------- */

int cerrarClientes(void);

int lanzarThreadServidor (pthread_t*servidor);
int lanzarThreadCliente (int newfd);

void* threadServidor (void* p);
void* threadAtenderCliente (void* pConexion);

/* --------------------------------------- variables globales -------------------------------------- */

ClientData listaConexiones [BACKLOG];
pthread_mutex_t mutexLista = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutexBufferCliente = PTHREAD_MUTEX_INITIALIZER;
char bufferCliente [BUFFER_CLIENTE_MAX];


/* --------------------------------------- funciones -------------------------------------- */

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
 * @fn int lanzarThreadServidor
 *
 * @brief funcion que lanza thread del servidor
 * 
 */

int lanzarThreadServidor (pthread_t*pServidor) {


    printf("Generando thread para iniciar Servidor\n");
    if(bloquearSign()) {
        perror("lanzarThreadServidor - error de bloquearSign()");
        return 1;
    }

    if(pthread_create (pServidor, NULL, threadServidor, NULL)) {
        perror("lanzarThreadServidor - pthread_create()");
        return 1;
    }

    if(desbloquearSign()) {
        perror("lanzarThreadServidor - error de desbloquearSign()");
        return 1;
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

int lanzarThreadCliente (int newfd) {

    int index;
    int clienteID;

    printf("Accediendo a lista de conexiones...");

    /* intento bloquear el acceso a la lista */
    if(pthread_mutex_lock (&mutexLista)) {
        close(newfd);    
        fprintf(stderr, "error al bloquear el acceso a la lista de conexiones");
        return 1;
    }
    index = cd_getFreeIndex(listaConexiones, BACKLOG);


    clienteID = listaConexiones[index].ID;
        
    //printf("cd_getFreeIndex return: %d\n", index);

    if(index == -1) {
        pthread_mutex_unlock (&mutexLista);
        close(newfd);
        fprintf(stderr, "memoria insuficiente para aceptar conexiones\n");
        return 1;
    }
    printf("lista de conexiones accedida\n");  


    if(bloquearSign()) {
        perror("Error de bloquearSign()");
        return 1;
    }

    printf("Generando thread para atender conexion...");
    if(pthread_create (&listaConexiones[index].thread, NULL, threadAtenderCliente, &listaConexiones[index])) {
        pthread_mutex_unlock (&mutexLista);
        perror("Error de pthread_create()");
        return 1;
    }

    if(pthread_detach(listaConexiones[index].thread)) {
        pthread_mutex_unlock (&mutexLista);
        perror("Error en pthread_detach()");
        return 1;
    }
    printf("hilo de atencion de cliente creado\n");

    listaConexiones[index].flagFree=0;
    listaConexiones[index].fd = newfd;

    if(pthread_mutex_unlock (&mutexLista)) {
        perror("Error al desbloquear el acceso a la lista de conexiones");
        return 1;
    }
    
    printf("\nSe establecio la conexion %4d\n\n", clienteID);



    if(desbloquearSign()) {
        perror("Error de desbloquearSign()");
        return 1;
    }

    // capturar sigint y poner el codigo de cerrar en la salida del accept
    // cerrar todos los threads de la lista que esten flag en cero
    // cerrar todos los sockets de la lista que esten flag en cero
    
    return 0;
}

/* --------------------------------------- threads -------------------------------------- */

/**
 * @fn void* lanzarServer (void* pConexion)
 *
 * @brief thread que inicia socket para conexion
 * 
 */


void* threadServidor (void* p) {

    socklen_t addr_len;
    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;

    char mensaje[MENSAJE_L];

    int newfd;

    // Creamos socket
    int s = socket(PF_INET,SOCK_STREAM, 0);

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
        return 0;
    }
    printf("se genero la direccion IP correctamente\n");


    // Abrimos puerto con bind()
    printf("Abriendo puerto...");    
    if (bind(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
        close(s);
        sprintf(mensaje, "Error al abrir el puerto %d", PORT);
        perror(mensaje);
        return 0;
    }
    printf("el puerto se abrio correctamente\n");

    // Seteamos socket en modo Listening
    printf("Configurando socket en modo <<listen>>...");
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


/**
 * @fn void* threadAtenderCliente (void* pConexion)
 *
 * @brief thread que se comunica con el cliente con un socket ya conectado.
 * 
 */

void* threadAtenderCliente (void* pConexion) {

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

        // Enviamos mensaje a cliente
	    if (write (newfd, buffer, strlen(buffer)) == -1)
	    {
      		perror("Thread: Error escribiendo mensaje en socket");
      		exit (1);
	    }




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



/**
 * @fn void* iniciarConexionSocket (void* param)
 *
 * @brief thread para 
 */
