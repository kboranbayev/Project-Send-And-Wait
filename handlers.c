#include "handlers.h"

void DieWithError(char *error)
{
	perror(error);
	exit(1);
}

void printReceived(struct sockaddr_in src, struct sockaddr_in dst, struct Packet *packet)
{
	char *type;
	switch(packet->PacketType) {
		case 0:
			type = "EOT";
			break;
		case 1:
			type = "ACK";
			break;
		case 2:
			type = "DATA";
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
			type = "ACK";
			break;
		case 2:
			type = "DATA";
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

int getWindowSize(char *totalData, int singlePacketSize)
{
	double n = (double)strlen(totalData)/singlePacketSize;
	return ceil(n);
}

void handleACK(struct sockaddr_in src, struct Packet packet, struct sockaddr_in dst, struct Packet *ack)
{
}

int generateSeqNum()
{
	return (rand() % 300);
}
