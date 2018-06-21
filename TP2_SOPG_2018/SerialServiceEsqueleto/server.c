/**
 * @file server.c
 *
 * @author Federico G. Roux (rouxfederico@gmail.com)
 *
 */

/* --------------------------------------- inclusion de archivos -------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>
#include <signal.h>

#include "ClientData.h"
#include "SerialManager.h"

#include "server.h"
#include "manejoThreads.h"

/* --------------------------------------- funciones -------------------------------------- */

int finalizarThreadServidor (pthread_t servidor_thread);

static int inicializarClientes(void);
static int cerrarClientes(void);
static int escribirSocketGeneral (int fd);

int lanzarThreadServidor (pthread_t*servidor);
int lanzarThreadCliente (int newfd);

void* threadServidor (void* p);
void* threadAtenderCliente (void* pConexion);

/* --------------------------------------- variables globales -------------------------------------- */

ClientData listaConexiones [BACKLOG];
pthread_mutex_t mutexLista = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutexBufferCliente = PTHREAD_MUTEX_INITIALIZER;
char bufferCliente [BUFFER_CLIENTE_MAX];

pthread_mutex_t mutexSocketGeneral = PTHREAD_MUTEX_INITIALIZER;
int socketGeneral;

/* --------------------------------------- funciones -------------------------------------- */


/**
 * @fn static int inicializarClientes(void);
 *
 * @brief inicializo estructura de datos todos los clientes
 * 
 */

static int inicializarClientes(void) {

    /* intento bloquear el acceso a la lista */
    if(pthread_mutex_lock (&mutexLista)) {
        fprintf(stderr, "error al bloquear el acceso a la lista de conexiones");
        return 1;
    }

    
    cd_init(listaConexiones, BACKLOG);

    if(pthread_mutex_unlock (&mutexLista)) {
        perror("Error al desbloquear el acceso a la lista de conexiones");
        return 1;
    }

    return 0;

}



/**
 * @fn static int cerrarClientes(void);
 *
 * @brief cierro todos los clientes
 * 
 */

static int cerrarClientes(void) {

    int index;

    /* intento bloquear el acceso a la lista */
    if(pthread_mutex_lock (&mutexLista)) {
        fprintf(stderr, "error al bloquear el acceso a la lista de conexiones");
        return 1;
    }


    for (index = 0; index < BACKLOG; index ++) {
        if(!listaConexiones[index].flagFree) {
            
            if(close(listaConexiones[index].fd)) {
                perror("cerrarClientes - Error de close()");
                return 1;
            }
            
            if(pthread_cancel(listaConexiones[index].thread)) {
                perror("cerrarClientes - Error de pthread_cancel()");
                return 1;
            }
            
        }
    }


   if(pthread_mutex_unlock (&mutexLista)) {
        perror("Error al desbloquear el acceso a la lista de conexiones");
        return 1;
    }

    return 0;
}
/**
 * @fn int lanzarThreadAtenderCIAA (pthread*pAtenderCIAA)
 *
 * @brief funcion que escribe variable global de socket a traves de mutex
 * 
 */


static int escribirSocketGeneral (int fd) {

    /* intento bloquear el acceso al socket gral */
    if(pthread_mutex_lock (&mutexSocketGeneral)) {
        fprintf(stderr, "error al bloquear el acceso al socket general");
        return 1;
    }

    socketGeneral = fd;

    /* intento desbloquear el acceso al socket gral */
    if(pthread_mutex_unlock (&mutexSocketGeneral)) {
        fprintf(stderr, "error al desbloquear el acceso al socket general");
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

int finalizarThreadServidor (pthread_t servidor_thread) {

    printf("Finalizando thread servidor\n");

    if(cerrarClientes()) {
        fprintf(stderr, "Error al intentar cerrar conexiones con clientes\n");
        return 1;
    }

    if(pthread_cancel (servidor_thread)) {
        perror("finalizarThreadAtenderCIAA - pthread_cancel()");
        return 1;
    }

    if(close(socketGeneral)) {
        perror("error al cerrar socketGeneral");
        return 1;
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

    /*
    if(bloquearSign()) {
        perror("Error de bloquearSign()");
        return 1;
    }
    */
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


    /*
    if(desbloquearSign()) {
        perror("Error de desbloquearSign()");
        return 1;
    }
    */
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

    char mensaje[MSJ_L];

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


    if(escribirSocketGeneral(s)) {
        fprintf(stderr, "No se pudo escribir el socket general. Saliendo...\n");
        exit(1);
    }

    printf("Inicializando estructura de datos...");
    if(inicializarClientes()) {
        fprintf(stderr, "No se pudo inicializar la lista de clientes\n");
        return 0;
    }
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

        lanzarThreadCliente(newfd);                 // una vez que estableci conexion, lanzo nuevo cliente  
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

