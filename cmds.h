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


void ls(char *cwd);
void cd(char *cwd, char *path);

#endif