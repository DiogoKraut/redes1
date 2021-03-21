#include "socket.h"

int errorDetection(tMessage *m) {
    unsigned char p;
    p = parity(m); // generate parity byte for current message
    if(p == m->parity) // check if parity byte matches
        return 1;
    else
        return 0;

}

unsigned char parity(tMessage *m) {
    unsigned char p1, p2;
    int i;

    p1 = (m->dest_addr << 6) | (m->src_addr << 4) | (m->size); // join fields into single byte
    p2 = (m->seq << 4) | (m->type); // join fields into single byte
    p1 = p1 ^ p2; //XOR

    for(i = 0; i < m->size; i++) {
        p1 = p1^ m->data[i]; // XOR with data field
    }
    return p1;
}

/* Builds a packet including all nescessary header info     *
 * ### Does not add terminating null byte to the data field */
void buildPacket(tMessage *mS, char *arg,  int type, int seq, int src, int dest) {
    /* Initial setup */
    mS->init = 0x7E;
    mS->dest_addr = dest;
    mS->src_addr  = src;
    mS->seq = seq;
    mS->size = 0;
    mS->data[0] = '\0';
    mS->type = type;

    if(arg != NULL) { /* Command has an argument */
        mS->size = strlen(arg);
        memcpy(mS->data, arg, mS->size);
    }
    mS->parity = parity(mS);

}

/* Sends the packet described in m and waits for the response   *
 * Returns 1 if response type matches the one specified in TYPE *
 * Returns 0 if response type is an error                       *
 * Resends if reponse type is NACK or TIMEOUT is reached        *
 *
 * mR must be valid even if calling program wont use it         */
int sendPacket(int socket, tMessage *mS, tMessage *mR, int TYPE) {
    int timeout = 0;
    unsigned char buffer[sizeof(tMessage)];
    send(socket, mS, sizeof(tMessage), 0);

    while(timeout < TIMEOUT_LIMIT) {
        /* recv response */
        recv(socket, &buffer, sizeof(tMessage), 0);
        memcpy(mR, &buffer, sizeof(tMessage));

        if(mR->init == 0x7E) {
            if(mR->type == TYPE)
                return 1;
            else if(mR->type == NACK)
                send(socket, mS, sizeof(tMessage), 0); // resend
            else if(mR->type == ERR) {
                packetError(mR->data[0]);
                return 0;
            }
        }
        timeout++;
    }
    /* TIMEOUT reached.. RESEND */
    printf("TIMEOUT reached.. RESEND %d\n", TYPE);
    return sendPacket(socket, mS, mR, TYPE);
}

void sendError(int socket, int err) {
    tMessage m;
    m.init = 0x7E;
    m.dest_addr = 0x1;
    m.src_addr  = 0x2;
    m.seq = 0;
    m.type = ERR;
    m.size = 1;
    m.data[1] = '\0';

    switch(err) {
        case EACCES:
            m.data[0] = PERM_DENIED;
            break;
        case ENOENT:
            m.data[0] = NO_FILE;
            break;
        case NO_LINE:
            m.data[0] = NO_LINE;
            break;
        case ENOTDIR:
            m.data[0] = NO_DIR;
            break;
    }
    send(socket, &m, sizeof(tMessage), 0);
}

void packetError(int e) {
    switch(e) {
        case PERM_DENIED:
            fprintf(stderr, "### ERR: Can't access file/directory. Permission Denied\n");
            break;
        case NO_DIR:
            fprintf(stderr, "### ERR: No such file/directory\n");
            break;
        case NO_FILE:
            fprintf(stderr, "### ERR: No such file/directory\n");
            break;
        case NO_LINE:
            fprintf(stderr, "### ERR: File doesn't contain specified line\n");
            break;
        default:
            fprintf(stderr, "### ERR: Undocumented error\n");
    }
}

int createSocket() {
    int sck;
    struct ifreq ir;
    struct sockaddr_ll addr;
    struct packet_mreq mr;

    sck = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));      /*cria socket*/
    if (sck == -1) {
        printf("Socket error\n");
        exit(-1);
    }

    memset(&ir, 0, sizeof(struct ifreq));      /*dispositivo lo*/
    strcpy(ir.ifr_name, "lo");
    if (ioctl(sck, SIOCGIFINDEX, &ir) == -1) {
        printf("ioctl error\n");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));       /*IP do dispositivo*/
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = 2; //ir.ifr_ifindex;
    if (bind(sck, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("bind error\n");
        exit(-1);
    }

    memset(&mr, 0, sizeof(mr));          /*Modo Promiscuo*/
    mr.mr_ifindex = ir.ifr_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(sck, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)    {
        printf("setsockopt error\n");
        exit(-1);
    }

    return sck;
}