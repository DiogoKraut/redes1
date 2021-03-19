#include "socket.h"

int errorDetection(tMessage *m) {
    unsigned char p;
    p = parity(m);
    // printf("0x%04X\n", p);
    if(p == m->parity)
        return 1;
    else
        return 0;

}

unsigned char parity(tMessage *m) {
    unsigned char p1, p2;
    p1 = (m->dest_addr << 6) | (m->src_addr << 4) | (m->size);
    p2 = (m->seq << 4) | (m->type);
    p1 = p1 ^ p2;
    return p1;
}

void buildPacket(tMessage *mS, char *arg,  int type, int seq) {
    /* Initial setup */
    mS->init = 0x7E;
    mS->dest_addr = 0x2;
    mS->src_addr  = 0x1;
    mS->seq = seq;
    mS->size = 0;
    mS->data[0] = '\0';

    mS->type = type;

    if(arg != NULL) { /* Command has an argument */
        mS->size = strlen(arg);
        memcpy(mS->data, arg, mS->size);
        mS->data[mS->size] = '\0';
    } else
        mS->size = 0;
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
                packetError(atoi(mR->data));
                return 0;
            }
        }
        timeout++;
    }
    /* TIMEOUT reached.. RESEND */
    printf("TIMEOUT reached.. RESEND\n");
    return sendPacket(socket, mS, mR, TYPE);
}

void packetError(int e) {
    switch(e) {
        case PERM_DENIED:
            fprintf(stderr, "### ERR: Can't access file/directory. Permission Denied\n");
            break;
        case NO_DIR:
            fprintf(stderr, "### ERR: No such directory\n");
            break;
        case NO_FILE:
            fprintf(stderr, "### ERR: No such file\n");
            break;
        case NO_LINE:
            fprintf(stderr, "### ERR: File doesn't contain specified line\n");
            break;
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

    memset(&addr, 0, sizeof(addr));   	  /*IP do dispositivo*/
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