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
    tMessage *mR = malloc(sizeof(tMessage));
    unsigned char *buffer = (unsigned char *) malloc(sizeof(tMessage));

    socket = createSocket();

	/* Get current working directoy */
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
	   perror("getcwd() error");
	   return 1;
	}

    while (1)
    {
    	printf("%s$\n", cwd);
    	if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
        	perror("### Error: Falha na recepção da mensagem");
        	exit(-1);
    	}
    	memcpy(mR, buffer, sizeof(tMessage));
    	if(mR->init == 0x7E) {


	    	if(mR->type == CMD_CD) {
	    		cd(cwd, mR->data);



				printf("%s$\n", cwd);

	    	}
    	}
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			perror("getcwd() error");
			return 1;
		}

    	printf("%s$\n", cwd);
    }


    free(buffer);
    free(m);
    return 0;
}