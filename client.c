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

	char c;
	char *cmd = NULL;
	char *tok;
	size_t len = 0;
	ssize_t read;
	char cwd[PATH_MAX]; // current working directory
	int end = 0;
	tMessage *m = malloc(sizeof(tMessage));

	socket = createSocket();
	/* Get current working directoy */
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
	   perror("getcwd() error");
	   return 1;
	}

	m->init = 0x7E;
	m->dest_addr = 0x2;
	m->src_addr = 0x1;
	m->size = 0x0;
	m->seq = 0x0;
	m->type = 0x1;

	unsigned char *buffer = (unsigned char *) malloc(sizeof(tMessage));
	memcpy(buffer, m, sizeof(tMessage));
	if (send(socket, buffer, 16, 0) < 0) {
        perror("Erro: Falha no envio da mensagem");
        return -1;
    }
	// /* Get command from user */
	// printf("%s:$", cwd);
	// while( end == 0 && (read = getline(&cmd, &len, stdin)) != -1) {
	// 	// if(cmd[0] != '\n') // not empty string
	// 		cmd[strcspn(cmd,"\n")] = '\0';

	// 	/* Get current working directoy */
	// 	if (getcwd(cwd, sizeof(cwd)) == NULL) {
	// 	   perror("getcwd() error");
	// 	   return 1;
	// 	}
	// 	tok = strtok(cmd, " ");

	// 	/* Run command */
	// 	if(tok != NULL) {
	// 		if(strcmp(tok, "lls") == 0) {
	// 			ls(cwd);
	// 		} else if(strcmp(tok, "lcd") == 0) {
	// 			tok = strtok(NULL, " ");
	// 			cd(cwd, tok);
	// 		} else if(strcmp(tok, "quit") == 0) {
	// 			end = 1;
	// 		} else {
	// 			// printf("Enter a command\n");
	// 		}

	// 	}
	// 	/* Update current working directory */
	// 	if (getcwd(cwd, sizeof(cwd)) == NULL) {
	// 		perror("getcwd() error");
	// 		return 1;
	// 	}
	// 	if(end == 0)
	// 		printf("%s:$", cwd);
	// }

	/* close */
	free(cmd);
}