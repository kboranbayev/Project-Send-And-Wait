/**
 * 
 * @author Kuanysh Boranbayev
 * @author Parm Dhaliwal
 * @date November 27, 2019
 */
#ifndef HANDLERS_H
#define HANDLERS_H

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "packet.h"

/*----------- Function Prototypes ------------------------*/
void DieWithError(char *error);

int sendPacket(int skt, struct Packet pkt, struct sockaddr_in dst);

struct Packet *receivePacket(int skt, struct sockaddr_in src);

struct PacketByte *receivePacketByte(int skt, struct sockaddr_in src);

char *getPacketType(int );

void printNetworkReceived(struct sockaddr_in src, struct sockaddr_in dst, struct Packet *packet);

void printNetworkTransmitted(struct sockaddr_in src, struct sockaddr_in dst, struct Packet packet);

void printNetworkReTransmitted(struct sockaddr_in src, struct sockaddr_in dst, struct Packet packet);

void printReceived(struct sockaddr_in src, struct sockaddr_in dst, struct Packet *packet);

void printReceivedRTT(struct sockaddr_in src, struct sockaddr_in dst, struct Packet *packet, int delay);

void printReceivedDuplicate(struct sockaddr_in src, struct sockaddr_in dst, struct Packet *packet);

void printTransmitted(struct sockaddr_in src, struct sockaddr_in dst, struct Packet packet);

void printReTransmitted(struct sockaddr_in src, struct sockaddr_in dst, struct Packet packet);

void printPacketDetail(struct Packet packet, int delay);

long delay (struct timeval t1, struct timeval t2);

long int getTotalPacketCount(char *totalData, int singlePacketSize);

long int setMaxWindowSize (long int server_size, long int client_size);

long int setMaxPacketCount (long int server_size, long int client_size);

int generateNum();

int rand100();

struct IP_PORT_ID *getIPsAndPorts(char *file);

#endif
