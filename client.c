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

            } else if((strcmp(cmd, "cd") == 0)) {
                buildPacket(mS, arg, CMD_CD, 0, CLIENT, SERVER);
                sendPacket(socket, mS, mR, ACK);

            } else if((strcmp(cmd, "ls") == 0)) {
                runCommand(socket, mS, mR, arg, CMD_LS, LS_DATA, CLIENT, SERVER);
                // seq = 0;             
                // buildPacket(mS, NULL, CMD_LS, 0);
                // sendPacket(socket, mS, mR, LS_DATA);

                // while(mR->type != EOTX && mR->type != ERR) {
                //     if(mR->type == LS_DATA) {
                //         if(errorDetection(mR)) {
                //             send(socket, ack, sizeof(tMessage), 0);
                //             memcpy(data_tmp, mR->data, mR->size);
                //             data_tmp[mR->size] = '\0';
                //             printf("%s\n", data_tmp);
                //         } else
                //             send(socket, nack, sizeof(tMessage), 0);
                // }
                // if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
                //     perror("### Err: Packet recv failed");
                //     exit(-1);
                // }
                // memcpy(mR, buffer, sizeof(tMessage));
                // }
                // if(mR->type == EOTX)
                // send(socket, ack, sizeof(tMessage), 0); //ACK EOTX
                // else if(mR->type == ERR)
                // packetError(mR->data[0]);

            } else if((strcmp(cmd, "ver") == 0)) {
                runCommand(socket, mS, mR, arg, CMD_CAT, CAT_DATA, CLIENT, SERVER);

                // seq = 0;
                // buildPacket(mS, arg, CMD_CAT, 0);
                // sendPacket(socket, mS, mR, CAT_DATA);

                // while(mR->type != EOTX && mR->type != ERR) { // process response until EOTX
                //     if(mR->type == CAT_DATA) {
                //         if(errorDetection(mR)) {
                //             send(socket, ack, sizeof(tMessage), 0);
                //             memcpy(data_tmp, mR->data, mR->size);
                //             data_tmp[mR->size] = '\0';
                //             printf("%d %s\n", seq, data_tmp); // print if no errors
                //             seq++;
                //         } else
                //             send(socket, nack, sizeof(tMessage), 0); // nack if error
                //     }
                //     /* Receive next response */
                //     if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
                //         perror("### Err: Packet recv failed");
                //         exit(-1);
                //     }
                //     memcpy(mR, buffer, sizeof(tMessage));
                // }
                // if(mR->type == EOTX)
                //     send(socket, ack, sizeof(tMessage), 0); //ACK EOTX
                // else if(mR->type == ERR)
                //     packetError(mR->data[0]);

            } else if((strcmp(cmd, "linha") == 0)) {
                buildPacket(mS, strtok(NULL, " "), CMD_LINE, 0, CLIENT, SERVER); //strtok return file name
                sendPacket(socket, mS, mR, ACK);// send file name, wait for ACK
                runCommand(socket, mS, mR, arg, CMD_CAT, CAT_DATA, CLIENT, SERVER);

                // buildPacket(mS, arg, LINE_DELIM, 0);
                // sendPacket(socket, mS, mR, CAT_DATA);// send line num, wait for the data
                // while(mR->type != EOTX  && mR->type != ERR) {
                //     if(mR->type == CAT_DATA) {
                //         if(errorDetection(mR)) {
                //             send(socket, ack, sizeof(tMessage), 0);
                //             memcpy(data_tmp, mR->data, mR->size);
                //             data_tmp[mR->size] = '\0';
                //             printf("%s\n", data_tmp);
                //         } else
                //             send(socket, nack, sizeof(tMessage), 0);
                //     }

                //     if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
                //         perror("### Err: Packet recv failed");
                //         exit(-1);
                //     }
                //     memcpy(mR, buffer, sizeof(tMessage));
                // }

                // if(mR->type == EOTX)
                //     send(socket, ack, sizeof(tMessage), 0); //ACK EOTX
                // else if(mR->type == ERR)
                //     packetError(mR->data[0]);
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
