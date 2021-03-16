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
	char cwd[PATH_MAX]; // current working directory
    tMessage *m = malloc(sizeof(tMessage));

    socket = createSocket();

    char *buffer = (char *) malloc(sizeof(tMessage));
    char *ack = (char *) malloc(sizeof(tMessage));
    char *nack = (char *) malloc(sizeof(tMessage));


	/* ACK and NACK setup */
	m->init = 0x7E;
	m->type = 0x8;
	m->size = 0;
	m->data[0] = '\0';
	memcpy(ack, m, sizeof(tMessage));
	m->type = 0x9;
	memcpy(nack, m, sizeof(tMessage));

	/* Get current working directoy */
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
	   perror("getcwd() error");
	   exit(-1);
	}

    while (1) {
    	if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
        	perror("### Err: Packet recv failed");
        	exit(-1);
    	}
    	memcpy(m, buffer, sizeof(tMessage));

    	/* if marker is found*/
    	if(m->init == 0x7E) {

    		// if(errorDetection(m))
    		// 	send(socket, ack, sizeof(tMessage), 0);
    		// else
    		// 	send(socket, nack, sizeof(tMessage),0);

    		/* interpret type and execute command */
    		switch(m->type) {
    			case CMD_CD:
	    			send(socket, ack, sizeof(tMessage), 0);
	    			cd(cwd, m->data);
					if (getcwd(cwd, sizeof(cwd)) == NULL) {
						perror("getcwd() error");
						exit(-1);
					}

					break;
				case CMD_LS:
					ls(socket);
					break;
    		}
    	}
    }



    free(buffer);
    free(m);
    free(ack);
    free(nack);

    return 0;
}
