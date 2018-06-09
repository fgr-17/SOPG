#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>

#include <string.h>

#include "ClientData.h"

#define CONEXIONES_MAX          100
#define BUFFER_CLIENTE_MAX      128

ClientData listaConexiones [CONEXIONES_MAX];


void* atenderCliente (void* pConexion)
{

	int n;
	char buffer[BUFFER_CLIENTE_MAX];

    ClientData* pData = (ClientData*) pConexion;

    int newfd = pData->fd;

    while(1)
    {

        if( (n =read(newfd,buffer,BUFFER_CLIENTE_MAX)) == -1 )
	    {
		    perror("Error leyendo mensaje en socket");
		    exit(1);
	    }
        
        if(n == 0)
            break;      // Se cerro la conexion

	    buffer[n]=0;
	    printf("Recibi %d bytes.:%s\n",n,buffer);

	    // Enviamos mensaje a cliente
	    if (write (newfd, buffer, strlen(buffer)) == -1)
	    {
      		perror("Error escribiendo mensaje en socket");
      		exit (1);
	    }
    }
	// Cerramos conexion con cliente
	close(newfd);
    // Levanto el flag.
    pData->flagFree = 1;

    printf("Termine el thread, espero un segundo\n");

	sleep(1);
}


int lanzarThreadCliente (int newfd)
{

    int index;

    index = cd_getFreeIndex(listaConexiones, CONEXIONES_MAX);
    if(index == -1)
    {
        close(newfd);
        printf("Memoria insuficiente para aceptar conexiones\n");
        return;
    }

    listaConexiones[index].flagFree=0;
    listaConexiones[index].fd = newfd;

    // bloquear signals
    // chequear error de pthread create
	pthread_create (&listaConexiones[index].thread, NULL, atenderCliente, &listaConexiones[index]);
       // errores de pthread detach
    pthread_detach(listaConexiones[index].thread);
    // desbloqeuar signals

    // hacer handler

    // capturar sigint y poner el codigo de cerrar en la salida del accept
    // cerrar todos los threads de la lista que esten flag en cero
    // cerrar todos los sockets de la lista que esten flag en cero
    
    return 0;
}

int main()
{
	socklen_t addr_len;
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;

	int newfd;

	// Creamos socket
	int s = socket(PF_INET,SOCK_STREAM, 0);

	// Cargamos datos de IP:PORT del server
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(4096);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(serveraddr.sin_addr.s_addr==INADDR_NONE)
    {
        fprintf(stderr,"ERROR invalid server IP\r\n");
        return 1;
    }

	// Abrimos puerto con bind()
	if (bind(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
		close(s);
		perror("listener: bind");
		return 1;
	}

	// Seteamos socket en modo Listening
	if (listen (s, 10) == -1) // backlog=10
  	{
    	perror("error en listen");
    	exit(1);
  	}

	while(1)
	{
		// Ejecutamos accept() para recibir conexiones entrantes
		addr_len = sizeof(struct sockaddr_in);
    	if ( (newfd = accept(s, (struct sockaddr *)&clientaddr, &addr_len)) == -1)
      	{
		      perror("error en accept");
		      exit(1);
	    }
	 	printf  ("server:  conexion desde:  %s\n", inet_ntoa(clientaddr.sin_addr));

        lanzarThreadCliente(newfd);

    
        /*
		// Leemos mensaje de cliente
		if( (n =read(newfd,buffer,128)) == -1 )
		{
			perror("Error leyendo mensaje en socket");
			exit(1);
		}
		buffer[n]=0;
		printf("Recibi %d bytes.:%s\n",n,buffer);

		// Enviamos mensaje a cliente
    	if (write (newfd, "hola", 5) == -1)
    	{
      		perror("Error escribiendo mensaje en socket");
      		exit (1);
    	}

		// Cerramos conexion con cliente
    	close(newfd);
        */
	}

	return 0;
}

