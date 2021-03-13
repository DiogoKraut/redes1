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

int main(void) {
	int socket;
	char *cmd = NULL;
	char *arg = NULL;
	char *temp;
	size_t len = 0;
	ssize_t read;
	char cwd[PATH_MAX]; // current working directory
	int end = 0;
	tMessage *mS = malloc(sizeof(tMessage));
	// tMessage *mR = malloc(sizeof(tMessage));

	socket = createSocket();
	/* Get current working directoy */
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
	   perror("getcwd() error");
	   return 1;
	}

	// m->init = 0x7E;
	// m->dest_addr = 0x2;
	// m->src_addr = 0x1;
	// m->size = 0x0;
	// m->seq = 0x0;
	// m->type = 0x1;
	// unsigned char *buffer = (unsigned char *) malloc(sizeof(tMessage));
	
	/* Get command from user */
	printf("%s:$", cwd);
	while( end == 0 && (read = getline(&temp, &len, stdin)) != -1) {
		temp[strcspn(temp,"\n")] = '\0';

		/* Get current working directoy */
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
		   perror("### ERR: getcwd() error");
		   exit(-1);
		}
		cmd = strtok(temp, " ");

		/* Run command */
		if(cmd != NULL) { /* Command isnt empty string */
			arg = strtok(NULL, " ");

		    if(strcmp(cmd, "lls") == 0) {
		        ls(cwd);

		    } else if(strcmp(cmd, "lcd") == 0) {
		        cd(cwd, cmd);


		    } else if(strcmp(cmd, "quit") == 0) {
		        end = 1;

		    } else if((strcmp(cmd, "ls") == 0)     || (strcmp(cmd, "cd") == 0)    ||
		    		  (strcmp(cmd, "ver") == 0)    || (strcmp(cmd, "linha") == 0) ||
		    		  (strcmp(cmd, "linhas") == 0) || (strcmp(cmd, "edit") == 0)) {
		    	buildPacket(mS, cmd, arg);

				if(sendPacket(socket, mS))
					printf("ACKED\n");
				else
					printf("###NACKED\n");

		    }


			/* Update current working directory */
			if (getcwd(cwd, sizeof(cwd)) == NULL) {
				perror("### ERR: getcwd() error");
				exit(-1);
			}
		}

		if(end == 0)
			printf("%s:$", cwd);
	}

	/* close */
	free(cmd);
}
