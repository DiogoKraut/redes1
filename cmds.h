#ifndef __CMDS__
#define __CMDS__

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
#include <dirent.h>
#include <math.h>
#include "socket.h"

void lls(void);
void lcd(char *cwd, char *path);
int cd(int socket, char *cwd, char *path);
int ls(int socket);
int cat(int socket, char *filename);
int line(int socket, char *filename, int line);
int lines(int socket, char *filename, int start, int end);
void runCommand(int socket, tMessage *mS, tMessage *mR, char *arg, int CMD_TYPE, int R_TYPE, int src, int dest);

#endif