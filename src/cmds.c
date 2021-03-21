#include "cmds.h"

/* Funtion sends a command and handles its response */
void runCommand(int socket, char *arg, int CMD_TYPE, int R_TYPE, int src, int dest, char *s) {
    int seq = 0;
    ssize_t ret;
    unsigned char *buffer = malloc(sizeof(tMessage));
    char *data_tmp = malloc(DATA_MAX+3);
    char *ack = malloc(sizeof(tMessage));
    char *nack = malloc(sizeof(tMessage));
    tMessage m, mS, mR;

    /* ACK and NACK setup */
    buildPacket(&m, NULL, ACK, 0, src, dest);
    memcpy(ack, &m, sizeof(tMessage));
    buildPacket(&m, NULL, NACK, 0, src, dest);
    memcpy(nack, &m, sizeof(tMessage));


    buildPacket(&mS, arg, CMD_TYPE, 0, src, dest);
    sendPacket(socket, &mS, &mR, R_TYPE);

    while(mR.type != EOTX && mR.type != ERR) { // process response until EOTX or ERR
        if(mR.type == R_TYPE && mR.seq == seq) {
            if(errorDetection(&mR)) {// print if no errors
                memcpy(data_tmp, mR.data, mR.size);
                data_tmp[mR.size] = '\0';
                if(s == NULL) {
                    printf("%s", data_tmp);
                }
                else {
                    strncat(s, data_tmp, mR.size);
                }
                send(socket, ack, sizeof(tMessage), 0);
            } else {
                send(socket, nack, sizeof(tMessage), 0); // nack if error
            }
            if(seq == SEQ_MAX)
                seq = 0; // there are more messages than seq can represent, reset to 0
            else
                seq++;
        }
        /* Receive next response */
        if ((ret = recv(socket, buffer, sizeof(tMessage), 0)) <= 0) {
            perror("### Err: Packet recv failed");
            exit(-1);
        }
        memcpy(&mR, buffer, sizeof(tMessage));
    }
    if(mR.type == EOTX)
        send(socket, ack, sizeof(tMessage), 0); //ACK EOTX
    else if(mR.type == ERR)
        // packetError(mR.data[0]);

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

int cd(int socket, char *cwd, char *path) {
    char temp[PATH_MAX];
    /* No path specified; return */
    if(path == NULL)
        return 0;

    /* If path isnt to Root('/'), "." or "..": add '/ to cwd and concatenate desired path'  */
    if( path[0] != '/' && (strcmp(path, ".") != 0) && (strcmp(path, "..") != 0) )  {
        strcpy(temp, cwd);
        strcat(temp, "/");
        strcat(temp, path);
    } else { // else: path is already a complete argument to chdir() 
        strcpy(temp, path);
    }

    if(chdir(temp) < 0) {
        sendError(socket, errno);
        return -1;
    }
    return 1;
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

    fclose(fp);
}
/* Function sends the name of files and directories in the cwd back to client          *
 * File names are contain '\n' at the end, unless its length is a multiple of DATA_MAX *
 * in which case '\n' is sent in a separate message                                    */
void ls(int socket) {
    struct dirent **namelist;
    int size, i, j, len;
    tMessage m;
    tMessage mR;

    /* Generate list of directories and files in cwd */
    size = scandir("./", &namelist, NULL, alphasort);
    if(size == -1) {
        sendError(socket, errno);
        return;
    }

    buildPacket(&m, NULL, LS_DATA, 0, SERVER, CLIENT);

    /* Iterate list and send each file/directory name separetly */
    for(i = 0; i < size; i++) {
        len = strlen(namelist[i]->d_name);
        /* Split longer file names and send consecutively */
        for(j = 0; j < floor(len / (float)DATA_MAX); j++) {
            m.size = DATA_MAX;
            memcpy(m.data, namelist[i]->d_name+(j*DATA_MAX), m.size);
            m.parity = parity(&m);
            if(!sendPacket(socket, &m, &mR, ACK)) {
                for(i = 0; i < size; i++) {
                    free(namelist[i]);
                }
                free(namelist);
                return;
            }
            m.seq++;
        }
        if(len%DATA_MAX == 0) { // EDGE CASE: len is a multiple of DATA_MAX
            m.size = 1;
            m.data[0] = '\n';
            m.parity = parity(&m);
            if(!sendPacket(socket, &m, &mR, ACK)) {
                for(i = 0; i < size; i++) {
                    free(namelist[i]);
                }
                free(namelist);
                return;
            }
            m.seq++;
        } else { // Leftover
            m.size = strlen(namelist[i]->d_name + (j*DATA_MAX))+1;
            memcpy(m.data, namelist[i]->d_name+(j*DATA_MAX), m.size-1);
            m.data[m.size-1] = '\n';
            m.parity = parity(&m);
            if(!sendPacket(socket, &m, &mR, ACK)) {
                for(i = 0; i < size; i++) {
                    free(namelist[i]);
                }
                free(namelist);
                return;
            }
            m.seq++;
        }
    }
    /* Send EOTX */
    buildPacket(&m, NULL, EOTX, 0, SERVER, CLIENT);
    sendPacket(socket, &m, &mR, ACK);

    for(i = 0; i < size; i++) {
        free(namelist[i]);
    }
    free(namelist);
}

/* Function sends the contents of a file line by line to the client *
 * Each line is preceded by its number                              */
void cat(int socket, FILE *fp) {
    tMessage m;
    tMessage mR;

    char *lineptr = NULL;
    size_t n = 0;
    ssize_t size;
    int j, count = 0;

    buildPacket(&m, NULL, CAT_DATA, 0, SERVER, CLIENT);

    while((size = getline(&lineptr, &n, fp)) != -1) {
        /* Sends line number first */
        m.size = 3;
        snprintf(m.data, m.size, "%01d ", count);
        m.parity = parity(&m);
        if(!sendPacket(socket, &m, &mR, ACK)) {
            free(lineptr);
            return;
        }
        m.seq++;
        /* Sends line content */
        for(j = 0; j < ceil(size / (float)DATA_MAX); j++) {
            m.size = strlen(lineptr+(j*DATA_MAX)) <= DATA_MAX ? strlen(lineptr+(j*DATA_MAX)) : DATA_MAX;
            memcpy(m.data, lineptr+(j*DATA_MAX), m.size);
            m.parity = parity(&m);
            if(!sendPacket(socket, &m, &mR, ACK)) {
                free(lineptr);
                return;
            }
            m.seq++;
        }
        count++;
    }
    /* Send EOTX */
    buildPacket(&m, NULL, EOTX, 0, SERVER, CLIENT);
    sendPacket(socket, &m, &mR, ACK);

    free(lineptr);
}

/* Sends the content of the specified line from fp to the client */
void line(int socket, FILE *fp, int line) {
    tMessage m;
    tMessage mR;
    char *lineptr = NULL;
    size_t n = 0;
    ssize_t size;
    int j, count = 0;

    buildPacket(&m, NULL, CAT_DATA, 0, SERVER, CLIENT);

    /* Read file line by line */
    while((size = getline(&lineptr, &n, fp)) != -1) {
        if(count == line) { // line found, send
            for(j = 0; j < ceil(size / (float)DATA_MAX); j++) {
                m.size = strlen(lineptr+(j*DATA_MAX)) <= DATA_MAX ? strlen(lineptr+(j*DATA_MAX)) : DATA_MAX;
                memcpy(m.data, lineptr+(j*DATA_MAX), m.size);
                m.parity = parity(&m);
                if(!sendPacket(socket, &m, &mR, ACK)) {
                    free(lineptr);
                    return;
            }
            m.seq++;
            }
        }
        count++;
    }

    if(count <= line) {// line doesnt exist
        sendError(socket, NO_LINE);
    } else {
        /* Send EOTX */
        buildPacket(&m, NULL, EOTX, 0, SERVER, CLIENT);
        sendPacket(socket, &m, &mR, ACK);
    }

    free(lineptr);
}

/* Funtion sends the content of lines numbered start to end from filename to client */
void lines(int socket, FILE *fp, int start, int end) {
    tMessage m;
    tMessage mR;
    char *lineptr = NULL;
    size_t n = 0;
    ssize_t size;
    int j, count = 0;

    buildPacket(&m, NULL, CAT_DATA, 0, SERVER, CLIENT);

    while((size = getline(&lineptr, &n, fp)) != -1) {
        if(count >= start && count <= end) {
            for(j = 0; j < ceil(size / (float)DATA_MAX); j++) {
            m.size = strlen(lineptr+(j*DATA_MAX)) <= DATA_MAX ? strlen(lineptr+(j*DATA_MAX)) : DATA_MAX;
            memcpy(m.data, lineptr+(j*DATA_MAX), m.size);
            m.parity = parity(&m);
            if(!sendPacket(socket, &m, &mR, ACK)) {
                free(lineptr);
                return;
            }
            m.seq++;
            }
        }
        count++;
    }

    if(count <= start) {// line doesnt exist
        sendError(socket, NO_LINE);
    } else {
        /* Send EOTX */
        buildPacket(&m, NULL, EOTX, 0, SERVER, CLIENT);
        sendPacket(socket, &m, &mR, ACK);
    }

    free(lineptr);
}

int edit(int socket, FILE *fp, int line, char *s) {
    // tMessage m;
    // tMessage mR;
    char *lineptr = NULL;
    size_t n = 0;
    ssize_t size;
    int i;
    FILE *replace;

    replace = fopen("replace.tmp", "w+");

    if(replace == NULL) {
        fprintf(stderr, "### edit: tmp file fopen error\n");
        return -1;
    }

    i = 0;
    while((size = getline(&lineptr, &n, fp)) != -1) {
        if(i == line) {
            fwrite(s, 1, strlen(s), replace);
        } else {
            fwrite(lineptr, 1, size, replace);
        }
        i++;
    }
    fclose(replace);

    return i >= line;
}