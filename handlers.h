#ifndef HANDLERS_H
#define HANDLERS_H

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <math.h>

#include "packet.h"

/*----------- Function Prototypes ------------------------*/
void DieWithError(char *error);

int sendPacket(int skt, struct Packet pkt, struct sockaddr_in dst);

struct PacketByte *receivePacket(int skt, struct sockaddr_in src);

void printReceived(struct sockaddr_in src, struct sockaddr_in dst, struct Packet *packet);

void printTransmitted(struct sockaddr_in src, struct sockaddr_in dst, struct Packet packet);

long delay (struct timeval t1, struct timeval t2);

int getWindowSize(char *totalData, int singlePacketSize);

int generateNum();

int rand100();

#endif
