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
	ssize_t ret;
	char *cmd = NULL;
	char *arg = NULL;
	char *temp;
	size_t len = 0;
	ssize_t read;
	char cwd[PATH_MAX]; // current working directory
	int end = 0;
	int seq;

	tMessage *mS = malloc(sizeof(tMessage));
	tMessage *mR = malloc(sizeof(tMessage));

	unsigned char *buffer = malloc(sizeof(tMessage));
	char *data_tmp = malloc(DATA_MAX+3);
    char *ack = malloc(sizeof(tMessage));
    char *nack = malloc(sizeof(tMessage));

	socket = createSocket();

	/* Get current working directoy */
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
	   perror("getcwd() error");
	   return 1;
	}

	/* ACK and NACK setup */
	mS->init = 0x7E;
	mS->type = ACK;
	mS->size = 0;
	mS->data[0] = '\0';
	memcpy(ack, mS, sizeof(tMessage));
	mS->type = NACK;
	memcpy(nack, mS, sizeof(tMessage));
	
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
		        lls();

		    } else if(strcmp(cmd, "lcd") == 0) {
		        cd(cwd, arg);


		    } else if(strcmp(cmd, "quit") == 0) {
		        end = 1;

		    } else if((strcmp(cmd, "cd") == 0)) {
		    	buildPacket(mS, arg, CMD_CD, 0);
				sendPacket(socket, mS, mR, ACK);

		    } else if((strcmp(cmd, "ls") == 0)) {
				seq = 0;		    	
		    	/* Treating first packet (file name) separetly		   *
		    	 * because sendPacket also receives the first response */
		    	buildPacket(mS, NULL, CMD_LS, 0);
				sendPacket(socket, mS, mR, LS_DATA);
				if(errorDetection(mR)) {
					send(socket, ack, sizeof(tMessage), 0);
					memcpy(data_tmp, mR->data, mR->size);
					data_tmp[mR->size] = '\0';
		    		printf("%s\n", data_tmp);
		    	} else
					send(socket, nack, sizeof(tMessage), 0);

				/* Receive rest of packets */
				do {
					if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
			        	perror("### Err: Packet recv failed");
			        	exit(-1);
			    	}
			    	memcpy(mR, buffer, sizeof(tMessage));

			    	if(mR->type == LS_DATA) {
			    		if(errorDetection(mR)) {
							send(socket, ack, sizeof(tMessage), 0);
				    		memcpy(data_tmp, mR->data, mR->size);
							data_tmp[mR->size] = '\0';
				    		printf("%s\n", data_tmp);
				    	} else
  							send(socket, nack, sizeof(tMessage), 0);
			    	}
				}while(mR->type != EOTX);
				send(socket, ack, sizeof(tMessage), 0); //ACK EOTX

		    } else if((strcmp(cmd, "ver") == 0)) {
		    	seq = 0;

		    	buildPacket(mS, arg, CMD_CAT, 0);
		    	sendPacket(socket, mS, mR, CAT_DATA);
		    	if(errorDetection(mR)) {
					send(socket, ack, sizeof(tMessage), 0);
		    		memcpy(data_tmp, mR->data, mR->size);
					data_tmp[mR->size] = '\0';
		    		printf("%d %s\n", seq, data_tmp);
		    		seq++;
		    	} else
					send(socket, nack, sizeof(tMessage), 0);

		    	do{ 
		    		if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
			        	perror("### Err: Packet recv failed");
			        	exit(-1);
			    	}
			    	memcpy(mR, buffer, sizeof(tMessage));

			    	if(mR->type == CAT_DATA) {
			    		if(errorDetection(mR)) {
							send(socket, ack, sizeof(tMessage), 0);
				    		memcpy(data_tmp, mR->data, mR->size);
							data_tmp[mR->size] = '\0';
				    		printf("%d %s\n", seq, data_tmp);
				    		seq++;
				    	} else
  							send(socket, nack, sizeof(tMessage), 0);
			    	}
				}while(mR->type != EOTX);
				send(socket, ack, sizeof(tMessage), 0); //ACK EOTX
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
    free(buffer);
    free(mS);
    free(mR);
    free(ack);
    free(nack);
}
