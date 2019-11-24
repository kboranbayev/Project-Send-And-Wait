#include "handlers.h"

void DieWithError(char *error)
{
	perror(error);
	exit(1);
}

int sendPacket(int skt, struct Packet pkt, struct sockaddr_in dst) 
{
    int n, dst_len = sizeof(dst);
    if ((n = sendto (skt, (struct Packet *)&pkt, sizeof(pkt), 0, (struct sockaddr *)&dst, dst_len)) == -1)
    {
        perror("TIMEOUT=>SEND");
        pkt.PacketType = 9;
    }

    return n;
}

struct Packet *receivePacket(int skt, struct sockaddr_in src)
{
    int n;
    unsigned int src_len = sizeof(src);
    struct Packet *pkt = malloc(sizeof(struct Packet));

    if ((n = recvfrom (skt, pkt, sizeof(*pkt), 0, (struct sockaddr *)&src, &src_len)) < 0)
    {
        perror("TIMEOUT=>GET");
        pkt->PacketType = 9;
    }
    
    return pkt;
}

struct PacketByte *receivePacketByte(int skt, struct sockaddr_in src)
{
    int n;
    unsigned int src_len = sizeof(src);
    struct PacketByte *result = malloc(sizeof(struct PacketByte));
    struct Packet *pkt = malloc(sizeof(struct Packet));
    if ((n = recvfrom (skt, pkt, sizeof(*pkt), 0, (struct sockaddr *)&src, &src_len)) < 0)
    {
        perror("TIMEOUT=>GET");
        pkt->PacketType = 9;
    }
    result->packet = *pkt;
    result->bytes = n;
    
    return result;
}

char *getPacketType(int PacketType) {
    char *type;
    switch(PacketType) {
		case 1:
			type = "SYN";
			break;
		case 2:
			type = "SYNACK";
			break;
        case 3:
            type = "ACK";
            break;
        case 4:
            type = "DATA";
            break;
        case 5:
            type = "NAK";
            break;
        case 8:
			type = "EOT";
			break;
        case 9:
            type = "TIMEO";
            break;
		default:
			exit(1);
	}
    return type;
}

void printNetworkReceived(struct sockaddr_in src, struct sockaddr_in dst, struct Packet *packet)
{
	char *type = getPacketType(packet->PacketType);
	
	printf(" Receive=>\tSrc = %s:%d\t\tDst = %s:%d\n\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\n", inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet->SeqNum, packet->AckNum, packet->WindowSize, packet->data);
}

void printNetworkTransmitted(struct sockaddr_in src, struct sockaddr_in dst, struct Packet packet)
{
	char *type = getPacketType(packet.PacketType);
	
    printf(" Transmit=>\tSrc = %s:%d\t\tDst = %s:%d\n\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\n",inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet.SeqNum, packet.AckNum, packet.WindowSize, packet.data);
}

void printNetworkReTransmitted(struct sockaddr_in src, struct sockaddr_in dst, struct Packet packet)
{
	char *type = getPacketType(packet.PacketType);

	printf(" Retransmit=>\tSrc = %s:%d\t\tDst = %s:%d\n\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\n",inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet.SeqNum, packet.AckNum, packet.WindowSize, packet.data);
}

void printReceived(struct sockaddr_in src, struct sockaddr_in dst, struct Packet *packet)
{
	char *type = getPacketType(packet->PacketType);
    FILE *fp;
    if (packet->PacketType == 2 || packet->PacketType == 3) {
        fp = fopen("logs/client_logs.txt", "a");
    } else {
        fp = fopen("logs/server_logs.txt", "a");
    }
    if (fp == NULL) {
        DieWithError("fopen ERROR");
    }

	
	printf(" Receive=>\tSrc = %s:%d\t\tDst = %s:%d\n\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\n", inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet->SeqNum, packet->AckNum, packet->WindowSize, packet->data);
    fprintf(fp, " Receive=>\tSrc = %s:%d\t\tDst = %s:%d\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\n", inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet->SeqNum, packet->AckNum, packet->WindowSize, packet->data);
    fclose(fp);
}

void printReceivedRTT(struct sockaddr_in src, struct sockaddr_in dst, struct Packet *packet, int delay) {
    char *type = getPacketType(packet->PacketType);
    FILE *fp;
    if (packet->PacketType == 2 || packet->PacketType == 3) {
        fp = fopen("logs/client_logs.txt", "a");
    } else {
        fp = fopen("logs/server_logs.txt", "a");
    }
    if (fp == NULL) {
        DieWithError("fopen ERROR");
    }
	
	printf(" Receive=>\tSrc = %s:%d\t\tDst = %s:%d\n\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\tRTT:%d ms\n", inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet->SeqNum, packet->AckNum, packet->WindowSize, packet->data, delay);
    fprintf(fp, " Receive=>\tSrc: %s:%d\t\tDst: %s:%d\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tRTT:%d ms\n", inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet->SeqNum, packet->AckNum, packet->WindowSize, delay);
    fclose(fp);
}

void printReceivedDuplicate(struct sockaddr_in src, struct sockaddr_in dst, struct Packet *packet)
{
	char *type = getPacketType(packet->PacketType);
    FILE *fp;
    if (packet->PacketType == 2 || packet->PacketType == 3) {
        fp = fopen("logs/client_logs.txt", "a");
    } else {
        fp = fopen("logs/server_logs.txt", "a");
    }
    if (fp == NULL) {
        DieWithError("fopen ERROR");
    }
	
	printf(" Duplicate=>\tSrc = %s:%d\t\tDst = %s:%d\n\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\n", inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet->SeqNum, packet->AckNum, packet->WindowSize, packet->data);
    fprintf(fp, " Duplicate=>\tSrc: %s:%d\t\tDst: %s:%d\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\n", inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet->SeqNum, packet->AckNum, packet->WindowSize, packet->data);
    fclose(fp);
}

void printTransmitted(struct sockaddr_in src, struct sockaddr_in dst, struct Packet packet)
{
	char *type = getPacketType(packet.PacketType);
    FILE *fp;
    if ((packet.PacketType == 1 || packet.PacketType == 4) || packet.PacketType == 8) {
        fp = fopen("logs/client_logs.txt", "a");
    } else {
        fp = fopen("logs/server_logs.txt", "a");
    }
    if (fp == NULL) {
        DieWithError("fopen ERROR");
    }
	
    printf(" Transmit=>\tSrc = %s:%d\t\tDst = %s:%d\n\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\n",inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet.SeqNum, packet.AckNum, packet.WindowSize, packet.data);
    fprintf(fp, " Transmit=>\tSrc: %s:%d\t\tDst: %s:%d\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\n",inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet.SeqNum, packet.AckNum, packet.WindowSize, packet.data);
    fclose(fp);
}

void printReTransmitted(struct sockaddr_in src, struct sockaddr_in dst, struct Packet packet)
{
	char *type = getPacketType(packet.PacketType);
    FILE *fp;
    if ((packet.PacketType == 1 || packet.PacketType == 4) || packet.PacketType == 8) {
        fp = fopen("logs/client_logs.txt", "a");
    } else {
        fp = fopen("logs/server_logs.txt", "a");
    }
    if (fp == NULL) {
        DieWithError("fopen ERROR");
    }

	printf(" Retransmit=>\tSrc = %s:%d\t\tDst = %s:%d\n\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\n",inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet.SeqNum, packet.AckNum, packet.WindowSize, packet.data);
    fprintf(fp, " Retransmit=>\tSrc: %s:%d\t\tDst: %s:%d\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\n",inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet.SeqNum, packet.AckNum, packet.WindowSize, packet.data);
    fclose(fp);
}

void printPacketDetail(struct Packet packet, int delay) {
    struct Packet pkt = packet;
    printf(" %d ms\t\t%d\t\t%d\t\t%d\t\t%s\t\t%d\n", delay, pkt.PacketType, pkt.SeqNum, pkt.AckNum, pkt.data, pkt.WindowSize);
}


// Compute the delay between tl and t2 in milliseconds 
long delay (struct timeval t1, struct timeval t2)
{
	long d;

	d = (t2.tv_sec - t1.tv_sec) * 1000;
	d += ((t2.tv_usec - t1.tv_usec + 500) / 1000);
	return(d);
}

int getTotalPacketCount (char *totalData, int singlePacketSize)
{
	double n = (double)strlen(totalData)/singlePacketSize;
	return ceil(n);
}

int setMaxWindowSize (int server_size, int client_size) {
    if (server_size > client_size) {
        return client_size;
    }
    return server_size;
}

int setMaxPacketCount (int server_size, int client_size) {
    if (server_size > client_size) {
        return server_size;
    }
    return client_size;
}

int generateNum()
{
	return (rand() % 300);
}

int rand100()
{
    return (rand() % 101);
}

struct IP_PORT_ID *getIPsAndPorts(char *file) {
    FILE *fp = fopen(file, "r");
    struct IP_PORT_ID *result = malloc(sizeof(struct IP_PORT_ID *));
    struct IP_PORT *ip_port = malloc(sizeof(struct IP_PORT *));
    int i = 0;
    char singleLine[100], *ip;
    while (!feof(fp) && i < 2) {
        fgets(singleLine, 100, fp);
        ip = strtok(singleLine, ":");
        strncpy(ip_port->ip_address, ip, sizeof(ip_port->ip_address));
        ip = strtok(NULL, "\n");
        ip_port->port = atoi(ip);
        result[i].ip_port = *ip_port;
        ++i;
        if (ip == NULL) {
            break;
        }
    }
    fclose(fp);

    return result;
}
