#include <stdio.h>
#include "ClientData.h"

#define CLIENTE_ID_BASE         0001

/**
 * @fn void cd_init(ClientData* clients, int len)
 *
 * @brief inicializo estructura de datos
 *
 */

void cd_init(ClientData* clients, int len)
{
	int i;
	for(i=0;i<len;i++) {
		clients[i].flagFree=1;
        clients[i].ID = CLIENTE_ID_BASE + i;
    }
}


/**
 * @fn int cd_getFreeIndex(ClientData* clients, int len)
 *
 * @brief devuelvo la primer estructura que tiene el buffer libre
 *
 */

int cd_getFreeIndex(ClientData* clients, int len)
{

   //  printf("Entre a free index\n\n");

	int i;
	for(i=0;i<len;i++)
	{
		if(clients[i].flagFree)
			return i;
	}
	return -1;
}


