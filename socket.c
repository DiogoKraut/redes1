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