
/* --------------------------------------- inclusion de archivos -------------------------------------- */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

/* --------------------------------------- definicion de constantes -------------------------------------- */


#define MSJ_SALIDA_SIGINT   "Recibi SIGINT. Saliendo..."
#define MSJ_BIENVENIDA          "Sistemas Operativos de Proposito General - CESE 2018\n------------------- Cliente TCP -------------------\n"

#define IP                      "127.0.0.1"
#define PORT                    (4096)

#define BUFFER_CLIENTE_MAX      (128)
#define MENSAJE_L               (100)
#define MSJ_SALIDA_SIGPIPE      "\nSe cerro el servidor. Saliendo... \n"

volatile sig_atomic_t fd;

/* --------------------------------------- signals -------------------------------------- */

/**
 * @fn void sigint_handler(int sig)
 *
 * @brief capturo SIGINT para el caso que quieran cerrar el programa con Ctrl+C
 */

void sigint_handler(int sig)
{
    write(0, MSJ_SALIDA_SIGINT, sizeof(MSJ_SALIDA_SIGINT));
    close(fd);
    exit(EXIT_SUCCESS);
}

/**
 * @fn void sigpipe_handler(int sig)
 *
 * @brief capturo SIGPIPE para el caso que si se cerro el reader y quiera escribir en la fifo
 */

void sigpipe_handler(int sig)
{
    write(0, MSJ_SALIDA_SIGPIPE, sizeof(MSJ_SALIDA_SIGPIPE));
    close(fd);
    exit(EXIT_FAILURE);
}

/* ---------------------------------------------- programa ppal : ----------------------------------------- */


/**
 * @fn int main(void)
 *
 * @brief prog. ppal. Creo un socket, escucho conexiones y atiendo con la funcion lanzarThreadCliente
 *        me quedo en el accept mientras en un thread se comunican con los clientes.
 * 
 */
int main()
{
    struct sockaddr_in serveraddr;
    struct sigaction si;
    struct sigaction sa;

    char mensaje [MENSAJE_L];
	char buf[BUFFER_CLIENTE_MAX];
    int n;
    

    printf(MSJ_BIENVENIDA);

    printf("\nIniciando el sistema...\n\n");


    printf("Instalando handlers de signals...\n");

    /* ------ instalo sigint ------ */
    //printf("Instalando handler de SIGINT...\n");
    si.sa_handler = sigint_handler;
    si.sa_flags = SA_RESTART;
    sigemptyset(&si.sa_mask);
    if (sigaction(SIGINT, &si, NULL) == -1) {
        perror("Error al instalar handler de SIGINT");
        exit(1);
    }
    printf("Handler de SIGINT instalado\n");

    /* ------ instalo sigpipe ------ */
    sa.sa_handler = sigpipe_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("Error al instalar handler de SIGPIPE");
        exit(1);
    }
    printf("Handler de SIGPIPE instalado\n");

	// Creamos socket
	fd = socket(PF_INET,SOCK_STREAM, 0);
    printf("\nGenerando socket para establecer conexion con server...");
	// Cargamos datos de direccion de server
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
    printf("Se genero la direccion IP correctamente\n");

	// Ejecutamos connect()
    printf("Estableciendo conexion con el Server...");
    if (connect(fd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        fprintf(stderr,"ERROR connecting\r\n");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("conexion establecida\n\n");

    while(1) {

	    // Enviamos mensaje a server
        printf("Ingrese un mensaje: \t");
        fgets(buf,sizeof(buf),stdin);

        n = write(fd, buf, strlen(buf));

        if(n == -1)
        {
            fprintf(stderr,"Error de write()\r\n");
            close(fd);
            exit(EXIT_FAILURE);
        }

	    // Leemos respuesta de server
        n = read(fd, buf, sizeof(buf));
        if(n>0)
        {
            buf[n] = 0;
            printf("Recibi:'%s'\r\n",buf);
        }

    }
	close(fd);

	return 0;
}

