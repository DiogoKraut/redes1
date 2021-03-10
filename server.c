#include <stdlib.h>
#include <unistd.h> 
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>

#include "cmds.h"
#include "socket.h"

int main() {
    int socket;
    ssize_t ret;
    tMessage *m = malloc(sizeof(tMessage));
    unsigned char *buffer = (unsigned char *) malloc(sizeof(tMessage));

    socket = createSocket();
	    

    while (1)
    {
    	if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
        	perror("### Error: Falha na recepção da mensagem");
        	exit(-1);
    	}
    	memcpy(m, buffer, sizeof(tMessage));
    	if(m->init == 0x7E) {
	    	printf("received\n");
	    	if(m->type == 0x1)
	    		printf("cd\n");
    	}

    }

    free(buffer);
    free(m);
    return 0;
}