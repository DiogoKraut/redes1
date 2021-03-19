#include "cmds.h"

void runCommand(int socket, tMessage *mS, tMessage *mR, char *arg, int CMD_TYPE, int R_TYPE, int src, int dest) {
    int seq = 0;
    ssize_t ret;
    unsigned char *buffer = malloc(sizeof(tMessage));
    char *data_tmp = malloc(DATA_MAX+3);
    char *ack = malloc(sizeof(tMessage));
    char *nack = malloc(sizeof(tMessage));
    tMessage m;

    /* ACK and NACK setup */
    buildPacket(&m, NULL, ACK, 0, src, dest);
    memcpy(ack, &m, sizeof(tMessage));
    buildPacket(&m, NULL, NACK, 0, src, dest);
    memcpy(nack, &m, sizeof(tMessage));


    buildPacket(mS, arg, CMD_TYPE, 0, src, dest);
    sendPacket(socket, mS, mR, R_TYPE);

    while(mR->type != EOTX && mR->type != ERR) { // process response until EOTX
        if(mR->type == R_TYPE) {
            if(errorDetection(mR)) {// print if no errors
                memcpy(data_tmp, mR->data, mR->size);
                data_tmp[mR->size] = '\0';
                if(mR->seq == 0) {
                    if(CMD_TYPE == CMD_CAT)
                        printf("%d %s", seq, data_tmp);
                    else
                        printf("%s\n", data_tmp);
                } else if(mR->seq == 1 && CMD_TYPE == CMD_CAT) {
                    printf("%d %s", seq, data_tmp);
                } else
                    printf("%s", data_tmp);



                seq++;
                send(socket, ack, sizeof(tMessage), 0);
            } else
                    send(socket, nack, sizeof(tMessage), 0); // nack if error
        }
        /* Receive next response */
        if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
            perror("### Err: Packet recv failed");
            exit(-1);
        }
        memcpy(mR, buffer, sizeof(tMessage));
    }
    if(mR->type == EOTX)
        send(socket, ack, sizeof(tMessage), 0); //ACK EOTX
    else if(mR->type == ERR)
        packetError(mR->data[0]);

    free(data_tmp);
    free(buffer);
    free(ack);
    free(nack);
}

void lcd(char *cwd, char *path) {
    char temp[PATH_MAX];

    /* No path specified; return */
    if(path == NULL)
        return;

    /* If path isnt to Root, "." or "..": add '/ to cwd and concatenate desired path'  */
    if( path[0] != '/' && (strcmp(path, ".") != 0) && (strcmp(path, "..") != 0) )  {
        strcpy(temp, cwd);
        strcat(temp, "/");
        strcat(temp, path);
    } else { // else: path is already a complete argument to chdir() 
        strcpy(temp, path);
    }

    if(chdir(temp) < 0) {
        printf("%s\n", strerror(errno));
    }
}

void cd(int socket, char *cwd, char *path) {
    char temp[PATH_MAX];
    /* No path specified; return */
    if(path == NULL)
        return;

    /* If path isnt to Root, "." or "..": add '/ to cwd and concatenate desired path'  */
    if( path[0] != '/' && (strcmp(path, ".") != 0) && (strcmp(path, "..") != 0) )  {
        strcpy(temp, cwd);
        strcat(temp, "/");
        strcat(temp, path);
    } else { // else: path is already a complete argument to chdir() 
        strcpy(temp, path);
    }

    if(chdir(temp) < 0) {
        sendError(socket, errno);
    }
}

void lls(void) {
    FILE *fp;
    char out[255];      // linux filename lenght limit
    char command[255] = "/bin/ls ./";

    /* Open ls process for reading */
    fp = popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }

    /* Read the output a line at a time - output it. */
    while (fgets(out, sizeof(out), fp) != NULL) {
        printf("%s", out);
    }

    pclose(fp);
}

int ls(int socket) {
    struct dirent **namelist;
    int size, i, j, len;
    tMessage m;
    tMessage mR;

    // char s[DATA_MAX+1];
    size = scandir("./", &namelist, NULL, alphasort);
    if(size == -1) {
        sendError(socket, errno);
        return -1;
    }

    buildPacket(&m, NULL, LS_DATA, 0, SERVER, CLIENT);

    for(i = 0; i < size; i++) {
        m.seq = 0;
        len = strlen(namelist[i]->d_name);
        for(j = 0; j < floor(len / DATA_MAX); j++) {
            m.seq++;
            m.size = DATA_MAX;
            memcpy(m.data, namelist[i]->d_name + (j*DATA_MAX), m.size);
            m.parity = parity(&m);
            // memcpy(s, m.data, m.size);
            if(!sendPacket(socket, &m, &mR, ACK))
                return -1;
            printf("ack received\n");
        }

        if(m.seq > 0) {
            m.seq++;
            m.size = strlen(namelist[i]->d_name + (j*DATA_MAX));
            memcpy(m.data, namelist[i]->d_name + (j*DATA_MAX), m.size);
            m.parity = parity(&m);
            if(!sendPacket(socket, &m, &mR, ACK))
                return -1;
            m.size = 1;
            m.data[0] = '\n';
            m.parity = parity(&m);
            if(!sendPacket(socket, &m, &mR, ACK))
                return -1;
        }
        m.size = strlen(namelist[i]->d_name + (j*DATA_MAX));
        memcpy(m.data, namelist[i]->d_name + (j*DATA_MAX), m.size);
        m.parity = parity(&m);
        if(!sendPacket(socket, &m, &mR, ACK))
            return -1;




        // m.size = strlen(namelist[i]->d_name) <= DATA_MAX ? strlen(namelist[i]->d_name) : DATA_MAX;
        // memcpy(m.data, namelist[i]->d_name, m.size);
        // m.parity = parity(&m);
        // if(!sendPacket(socket, &m, &mR, ACK))
        //     return -1;




    }
    /* Send EOTX */
    buildPacket(&m, NULL, EOTX, 0, SERVER, CLIENT);
    sendPacket(socket, &m, &mR, ACK);

    for(i = 0; i < size; i++) {
        free(namelist[i]);
    }
    free(namelist);

    return 1;
}

int cat(int socket, char *filename) {
    tMessage m;
    tMessage mR;

    FILE *fp;
    char *lineptr = NULL;
    size_t n = 0;
    ssize_t size;

    fp = fopen(filename, "r");
    if(fp == NULL) {
        sendError(socket, errno);
        return 0;
    }

    buildPacket(&m, NULL, CAT_DATA, 0, SERVER, CLIENT);

    while((size = getline(&lineptr, &n, fp)) != -1) {
        m.size = strlen(lineptr) <= DATA_MAX ? strlen(lineptr) : DATA_MAX;
        memcpy(m.data, lineptr, m.size);
        // m.data[m.size-1] = '\0';
        m.parity = parity(&m);
        if(!sendPacket(socket, &m, &mR, ACK))
            return -1;
    }
    /* Send EOTX */
    buildPacket(&m, NULL, EOTX, 0, SERVER, CLIENT);
    sendPacket(socket, &m, &mR, ACK);

    fclose(fp);
    free(lineptr);
    return 1;

}

int line(int socket, char *filename, int line) {
    tMessage m;
    tMessage mR;
    char *lineptr = NULL;
    size_t n = 0;
    ssize_t size;
    int count = 0;

    FILE *fp;

    fp = fopen(filename, "r");
    if(fp == NULL) {
        sendError(socket, errno);
        return 0;
    }

    buildPacket(&m, NULL, CAT_DATA, 0, SERVER, CLIENT);

    while((size = getline(&lineptr, &n, fp)) != -1) {
        if(count == line) {
            m.size = strlen(lineptr) <= DATA_MAX ? strlen(lineptr) : DATA_MAX;
            memcpy(m.data, lineptr, m.size);
            // m.data[m.size-1] = '\0';
            m.parity = parity(&m);
            if(!sendPacket(socket, &m, &mR, ACK)) {
                free(lineptr);
                return -1;
            }
        }
        count++;
    }

    if(count <= line) {// line doesnt exist
        sendError(socket, NO_LINE);
        free(lineptr);
        return 0;
    } else {
        /* Send EOTX */
        buildPacket(&m, NULL, EOTX, 0, SERVER, CLIENT);
        sendPacket(socket, &m, &mR, ACK);
    }

    fclose(fp);
    free(lineptr);

    return 0;
}