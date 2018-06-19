#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "SerialManager.h"


#define BAUDRATE                115200
#define TTY                     1
#define TTY_TEXTO               "/dev/ttyUSB1"
#define SERIAL_REC_BUF_L        128

#define ERROR_SERIAL_OPEN       1

//tener en cuenta sigint - sigpipe
//enmascarar signals en un solo thread

int main(void)
{

    char serial_rec_buf [SERIAL_REC_BUF_L];
    int bytes_recibidos;


    printf("Inicio Serial Service\r\n");
	
    /* abro puerto serie */
    if(serial_open(TTY, BAUDRATE)) {
        perror("serial_open");
        return ERROR_SERIAL_OPEN;        
    }
    printf("Puerto serie abierto: %s %d 8N1\n", TTY_TEXTO, BAUDRATE);
    //lanzar thread server tcp()

    while(1)
    {
        bytes_recibidos  = serial_receive(serial_rec_buf, SERIAL_REC_BUF_L);

        if(bytes_recibidos < 0) {
            perror("serial_receive");
        }
        else if(bytes_recibidos > 0)
            printf("%s", serial_rec_buf);
        
        usleep(100000);
    }






    exit(EXIT_SUCCESS);
    return 0;
}
