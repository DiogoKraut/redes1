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
    FILE *fp;
    int socket;
    ssize_t ret;
    char cwd[PATH_MAX]; // current working directory
    tMessage *m = malloc(sizeof(tMessage));
    tMessage *mR = malloc(sizeof(tMessage));

    socket = createSocket();

    char *buffer = malloc(sizeof(tMessage));
    char *tmp = malloc(DATA_MAX+1);
    char edit_data[EDIT_MAX];

    char *ack    = malloc(sizeof(tMessage));
    char *nack   = malloc(sizeof(tMessage));
    int edit_line;

    /* ACK and NACK setup */
    m->init = 0x7E;
    m->type = ACK;
    m->size = 0;
    m->data[0] = '\0';
    memcpy(ack, m, sizeof(tMessage));
    m->type = NACK;
    memcpy(nack, m, sizeof(tMessage));

    /* Get current working directoy */
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
       perror("getcwd() error");
       exit(-1);
    }
    setbuf(stdout, NULL);
    printf("%s", cwd);

    while (1) {
        if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
            perror("### Err: Packet recv failed");
            exit(-1);
        }
        memcpy(m, buffer, sizeof(tMessage));

        /* if marker is found*/
        if(m->init == 0x7E) {
            /* Copy to temporary buffer to add terminating null byte */
            if(m->size > 0) {
                memcpy(tmp, m->data, m->size);
                tmp[m->size] = '\0';
            }
            /* Interpret type and execute command */
            switch(m->type) {
                case CMD_CD:
                    if(cd(socket, cwd, tmp) == 1) { // directory change was sucessful
                        send(socket, ack, sizeof(tMessage), 0);
                        if (getcwd(cwd, sizeof(cwd)) == NULL) { // update cwd
                            perror("getcwd() error");
                            exit(-1);
                        }
                        printf("\n%s", cwd);
                    }
                    break;

                case CMD_LS:
                    ls(socket);
                    break;

                case CMD_CAT:
                    fp = fopen(tmp, "r+");
                    if(fp == NULL) {
                        sendError(socket, errno);
                    } else {
                        send(socket, ack, sizeof(tMessage), 0);
                        cat(socket, fp);
                        fclose(fp);
                    }
                    break;

                case CMD_LINE:
                    fp = fopen(tmp, "r+");
                    if(fp == NULL) {
                        sendError(socket, errno);
                    } else {
                        send(socket, ack, sizeof(tMessage), 0);
                        while(m->type != LINE_DELIM) { // wait for message with line number
                            if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
                                perror("### Err: Packet recv failed");
                                exit(-1);
                            }
                            memcpy(m, buffer, sizeof(tMessage));
                        }
                        line(socket, fp, m->data[0] - '0');
                        fclose(fp);
                    }
                    break;

                case CMD_LINES:
                    fp = fopen(tmp, "r+");
                    if(fp == NULL) {
                        sendError(socket, errno);
                    } else {
                        send(socket, ack, sizeof(tMessage), 0);
                        memcpy(tmp, m->data, m->size);
                        tmp[m->size] = '\0';
                        while(m->type != LINE_DELIM) { // wait for message with line numbers
                            if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
                                perror("### Err: Packet recv failed");
                                exit(-1);
                            }
                            memcpy(m, buffer, sizeof(tMessage));
                        }
                        lines(socket, fp, m->data[0] - '0', m->data[1] - '0'); //convert data to int in arguments
                        fclose(fp);
                    }
                    break;

                case CMD_EDIT:
                    edit_data[0] = '\0';
                    fp = fopen(tmp, "r+");
                    if(fp == NULL) {
                        sendError(socket, errno);
                    } else {
                        send(socket, ack, sizeof(tMessage), 0);
                        while(m->type != LINE_DELIM) {// wait for message with line numbers
                            if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
                                perror("### Err: Packet recv failed");
                                exit(-1);
                            }
                            memcpy(m, buffer, sizeof(tMessage));
                        }
                        edit_line = m->data[0] - '0';//conver to int
                        runCommand(socket, NULL, ACK, CAT_DATA, SERVER, CLIENT, edit_data);  
                        strcat(edit_data, "\n"); //lines are terminated by \n
                        edit(socket, fp, edit_line, edit_data);               
                        fclose(fp);

                        remove(tmp); //delete original file
                        rename("replace.tmp", tmp); //rename file replacement to original name
                    }
                    break;
            }
        }
    }



    free(buffer);
    free(m);
    free(mR);
    free(ack);
    free(nack);

    return 0;
}
