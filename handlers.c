#include "handlers.h"

void DieWithError(char *error)
{
	perror(error);
	exit(1);
}

int sendPacket(int skt, struct Packet pkt, struct sockaddr_in dst) 
{
    int n, dst_len = sizeof(dst);
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    setsockopt(skt, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if ((n = sendto (skt, (struct Packet *)&pkt, sizeof(pkt), 0, (struct sockaddr *)&dst, dst_len)) == -1)
    {
        perror("TIMEOUT=>SEND");
        pkt.PacketType = 99;
    }

    return n;
}

struct PacketByte *receivePacket(int skt, struct sockaddr_in src)
{
    int n;
    unsigned int src_len = sizeof(src);
    struct PacketByte *result = malloc(sizeof(struct PacketByte));
    struct Packet *pkt = malloc(sizeof(struct Packet));
    struct timeval timeout;
    timeout.tv_sec = 60;
    timeout.tv_usec = 0;
    setsockopt(skt, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if ((n = recvfrom (skt, pkt, sizeof(*pkt), 0, (struct sockaddr *)&src, &src_len)) < 0)
    {
        perror("TIMEOUT=>GET");
        pkt->PacketType = 99;
    }
    result->packet = *pkt;
    result->bytes = n;
    
    return result;
}

void printReceived(struct sockaddr_in src, struct sockaddr_in dst, struct Packet *packet)
{
	char *type;
	switch(packet->PacketType) {
		case 0:
			type = "EOT";
			break;
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
        case 99:
            type = "TIMEO";
            break;
		default:
			exit(1);
	}
	printf(" Receive=>\tSource = %s:%d\t\tDestination = %s:%d\n\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\n", inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet->SeqNum, packet->AckNum, packet->WindowSize, packet->data);
}

void printTransmitted(struct sockaddr_in src, struct sockaddr_in dst, struct Packet packet)
{
	char *type;
	switch(packet.PacketType) {
		case 0:
			type = "EOT";
			break;
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
        case 99:
            type = "TIMEO";
            break;
		default:
			exit(1);
	}
	printf(" Transmit=>\tSource = %s:%d\t\tDestination = %s:%d\n\t\tPacketType = %s\tSeqNum = %d\tAckNum: %d\tWindowSize = %d\tdata: %s\n",inet_ntoa(src.sin_addr), src.sin_port, inet_ntoa(dst.sin_addr), dst.sin_port, type, packet.SeqNum, packet.AckNum, packet.WindowSize, packet.data);
}

// Compute the delay between tl and t2 in milliseconds 
long delay (struct timeval t1, struct timeval t2)
{
	long d;

	d = (t2.tv_sec - t1.tv_sec) * 1000;
	d += ((t2.tv_usec - t1.tv_usec + 500) / 1000);
	return(d);
}

int getWindowSize (char *totalData, int singlePacketSize)
{
	double n = (double)strlen(totalData)/singlePacketSize;
	return ceil(n);
}

int generateNum()
{
	return (rand() % 300);
}

int rand100()
{
    return (rand() % 101);
}
