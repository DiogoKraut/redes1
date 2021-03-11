#ifndef __SOCKET__
#define __SOCKET__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

typedef struct sMessage {
	unsigned char init;

	uint8_t dest_addr : 2;
	uint8_t src_addr  : 2;
	uint8_t size      : 4;

	uint8_t seq  : 4;
	uint8_t type : 4;

	char data[15];

	unsigned char parity;
} tMessage;

int createSocket();
int errorDetection(tMessage *m);
unsigned char parity(tMessage *m);

#endif