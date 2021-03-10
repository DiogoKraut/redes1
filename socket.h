#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <arpa/inet.h>

#define CMD_CD     0x0
#define CMD_LS     0x1
#define CMD_CAT    0X2
#define CMD_LINE   0X3
#define CMD_LINES  0X4
#define CMD_EDIT   0X5
#define ACK        0X8
#define	NACK       0X9
#define LINE_DELIM 0XA
#define	LS_DATA    0XB
#define CAT_DATA   0XC
#define EOTX       0XD
#define ERR        0XF

int createSocket();

typedef struct sMessage {
	unsigned char init;

	uint8_t dest_addr : 2;
	uint8_t src_addr  : 2;
	uint8_t size      : 4;

	uint8_t seq  : 4;
	uint8_t type : 4;

	unsigned char data[15];

	unsigned char parity;
} tMessage;

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

// int sendMessage(char type, )