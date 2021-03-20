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
    char *argB = NULL;
    char *temp;
    size_t len = 0;
    ssize_t read;
    char cwd[PATH_MAX]; // current working directory
    int end = 0;

    tMessage *mS = malloc(sizeof(tMessage));
    tMessage *mR = malloc(sizeof(tMessage));

    // unsigned char *buffer = malloc(sizeof(tMessage));
    // char *data_tmp = malloc(DATA_MAX+3);
    // char *ack = malloc(sizeof(tMessage));
    // char *nack = malloc(sizeof(tMessage));

    socket = createSocket();

    /* Get current working directoy */
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return 1;
    }

    /* ACK and NACK setup */
    // mS->init = 0x7E;
    // mS->type = ACK;
    // mS->size = 0;
    // mS->data[0] = '\0';
    // memcpy(ack, mS, sizeof(tMessage));
    // mS->type = NACK;
    // memcpy(nack, mS, sizeof(tMessage));
    
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
                lcd(cwd, arg);
                /* Update current working directory */
                if (getcwd(cwd, sizeof(cwd)) == NULL) {
                    printf("### ERR: getcwd() error");
                    exit(-1);
                }

            } else if(strcmp(cmd, "quit") == 0) {
                end = 1;

            } else if(strcmp(cmd, "cd") == 0) {
                buildPacket(mS, arg, CMD_CD, 0, CLIENT, SERVER);
                sendPacket(socket, mS, mR, ACK);

            } else if(strcmp(cmd, "ls") == 0) {
                runCommand(socket, mS, mR, arg, CMD_LS, LS_DATA, CLIENT, SERVER);

            } else if(strcmp(cmd, "ver") == 0) {
                runCommand(socket, mS, mR, arg, CMD_CAT, CAT_DATA, CLIENT, SERVER);

            } else if(strcmp(cmd, "linha") == 0) {
                if((atoi(arg) >= 0) && (atoi(arg) <= 9)) {
                    buildPacket(mS, strtok(NULL, " "), CMD_LINE, 0, CLIENT, SERVER); //strtok returns file name
                    sendPacket(socket, mS, mR, ACK);// send file name, wait for ACK
                    runCommand(socket, mS, mR, arg, LINE_DELIM, CAT_DATA, CLIENT, SERVER);
                } else
                    fprintf(stderr, "### ERR: LINE must be between 0 and 9\n");
            } else if(strcmp(cmd, "linhas") == 0) {
                argB = strtok(NULL, " "); // set final line
                if(atoi(arg) >= 0 && atoi(arg) <= 9 && atoi(argB) >= 0 && atoi(argB) <= 9) {
                    buildPacket(mS, strtok(NULL, " "), CMD_LINES, 0, CLIENT, SERVER); //strtok returns file name
                    sendPacket(socket, mS, mR, ACK);// send file name, wait for ACK
                    strncat(arg, argB, 1);
                    runCommand(socket, mS, mR, arg, LINE_DELIM, CAT_DATA, CLIENT, SERVER);
                } else
                    fprintf(stderr, "### ERR: LINE must be between 0 and 9\n");
            }
        }

        if(end == 0)
            printf("%s:$", cwd);
    }

    /* close */
    // free(buffer);
    free(mS);
    free(mR);
    // free(ack);
    // free(nack);
}
