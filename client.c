// A simple UCP client program allows the user to specify a server by its domain name.

#include "handlers.h"

#define CLIENT_UDP_PORT        6000    // Default client port
#define SERVER_UDP_PORT		   7000	   // Default server port
#define WIN				       65000   // Default Window Size
#define DEFLEN				   64	   // Default Length

int main (int argc, char **argv)
{
	int port = SERVER_UDP_PORT;
	int sd;
	char *pname, *host;
	struct	hostent	*hp;
	struct	sockaddr_in server, client;
	struct  timeval start, end;
    
	pname = argv[0];
	argc--;
	argv++;
	if (argc > 0 && (strcmp(*argv, "-s") == 0))
	{
		if (--argc > 0)
		{
			argc--;
			argv++;
		}
		else
		{
			fprintf (stderr,"Usage: %s [-s packet size] host [port]\n", pname);
			exit(1);
		}
	}
	if (argc > 0) 
	{
		host = *argv;
			if (--argc > 0)
				port = atoi(*++argv);
	}
	else
	{
		fprintf(stderr, "Usage:	%s [-s packet size] host [port]\n", pname);
		exit(1);
	}

	// Create a datagram socket
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		DieWithError ("Can't create a socket");
	}

	// Store server's information
	memset((char *)&server, 0, sizeof(server)); 
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if ((hp = gethostbyname(host)) == NULL)
	{
		DieWithError ("Can't get server's IP address");
	}
	bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);

	// Bind local address to the socket 
	memset((char *)&client, 0, sizeof(client)); 
	client.sin_family = AF_INET; 
	client.sin_port = htons(CLIENT_UDP_PORT);
	client.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(sd, (struct sockaddr *)&client, sizeof(client)) == -1)
	{
		DieWithError ("Can't bind name to socket");
	}
	
        
    // three-way handshake starts
    struct Packet syn_packet;
    
    syn_packet.PacketType = 1;
    syn_packet.SeqNum = 100;
    syn_packet.AckNum = 200;
    syn_packet.WindowSize = WIN;
    memset(syn_packet.data, 0, sizeof(syn_packet.data));
    
    //struct PacketByte *pktByte = malloc(sizeof(struct PacketByte));
    struct Packet *syn_ack_packet = malloc(sizeof(struct Packet));
    
    int n, server_len = sizeof(server);
    
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    // start RTT calculator
    gettimeofday(&start, NULL);
    
    // 2 way handshake session
    while (1) {
        // send SYN
        sendPacket (sd, syn_packet, server);
        printTransmitted (client, server, syn_packet);
        // receive SYNACK
        while ((n = recvfrom (sd, syn_ack_packet, sizeof(*syn_ack_packet), 0, (struct sockaddr *)&server, (unsigned int *)&server_len)) < 0) {
            sendPacket (sd, syn_packet, server);
            printReTransmitted( client, server, syn_packet);
        }
        printReceived (server, client, syn_ack_packet);
        break;
    }
    
    gettimeofday (&end, NULL);
    printf ("Round-trip delay = %ld ms.\n", delay(start, end));
    // two-way handshake ends

    //char *msg = "The objective of this project is to design and implement a basic Send-And-Wait protocol simulator. The protocol will be half-duplex and use sliding windows to send multiple packets between two hosts on a LAN with an \"unrealiable network\" between the two hosts.";
    
    char *msg = "The objective of this project is to design and implement a basic Send-And-Wait protocol simulator. The protocol will be half-duplex and use sliding windows to send multiple packets between two hosts on a LAN with an \"unrealiable network\" between the two hosts. The following diagram depicts the model:\nYour Mission\n - You may use any language of your choice to implement the three components shown in the diagram above. It is strongly recommended that you use your code from the first assignment to implement the peer stations.\n - You will be designing an application layer protocol in this case on top of UDP (in keeping with the wireless channel model). The protocol should be able to handle network errors such as packet loss and duplicate packets. You will implement timeouts and ACKs to handle retransmissions due to lost packets (ARQ).";
    int packet_counter = 0, total_packet_count = 0, acked_packet_count = 0, windowSize_counter = syn_ack_packet->WindowSize - 1;
    int shift = 0;
    
    struct Packet data_packet;
    struct Packet copy_packet;
    struct Packet transmitted_packets[syn_ack_packet->WindowSize];
    struct PacketRTT *acked_packets[getWindowSize(msg, sizeof(syn_ack_packet->data))];

    struct Packet *ack = malloc(sizeof(struct Packet));

	// transmit data


    // old
    // while (total_packet_count < getWindowSize(msg, sizeof(syn_ack_packet->data))) {
    //     gettimeofday(&start, NULL);
    //     while (packet_counter < syn_ack_packet->WindowSize) {
    //         data_packet.PacketType = 4;
    //         printf("size = %d\n", getWindowSize(msg, sizeof(syn_ack_packet->data)));
    //         if (copy_packet.SeqNum != 0 && total_packet_count < getWindowSize(msg, sizeof(syn_ack_packet->data))) {
    //             data_packet.SeqNum = copy_packet.SeqNum +  + getWindowSize(msg, sizeof(syn_ack_packet->data));
    //             data_packet.AckNum = copy_packet.AckNum +  + getWindowSize(msg, sizeof(syn_ack_packet->data));
    //             data_packet.WindowSize = windowSize_counter;
    //             char *to = (char *) malloc(sizeof(data_packet.data));
    //             strncpy(to, msg + shift, sizeof(data_packet.data));
    //             strncpy(data_packet.data, to, sizeof(data_packet.data));
    //         } else if (copy_packet.SeqNum == 0 && total_packet_count < getWindowSize(msg, sizeof(syn_ack_packet->data))) {
    //             data_packet.SeqNum = syn_ack_packet->AckNum;
    //             data_packet.AckNum = syn_ack_packet->SeqNum + getWindowSize(msg, sizeof(syn_ack_packet->data)); // idk
    //             data_packet.WindowSize = windowSize_counter;
    //             strncpy(data_packet.data, msg, sizeof(data_packet.data));
    //         } else if (total_packet_count == getWindowSize(msg, sizeof(syn_ack_packet->data))) { // 
    //             data_packet.PacketType = 8; // EOT
    //             data_packet.SeqNum = copy_packet.SeqNum +  + getWindowSize(msg, sizeof(syn_ack_packet->data));
    //             data_packet.AckNum = copy_packet.AckNum +  + getWindowSize(msg, sizeof(syn_ack_packet->data));
    //             data_packet.WindowSize = windowSize_counter;
    //             memset(data_packet.data, 0, sizeof(data_packet.data));
    //         }

    //         if (windowSize_counter == 0) {
    //             windowSize_counter = syn_ack_packet->WindowSize;
    //         }

    //         if (copy_packet.PacketType == 8) {
    //             printf("EOT sent\n");
    //             break;
    //         } 

    //         sendPacket (sd, data_packet, server);
    //         printTransmitted (client, server, data_packet);
            
    //         transmitted_packets[packet_counter] = data_packet;
    //         copy_packet = data_packet;
    //         packet_counter++;
    //         total_packet_count++;
    //         windowSize_counter--;
    //         shift += sizeof(data_packet.data);
    //     }
        
        
    //     while (packet_counter > 0) {
    //         ack = (struct Packet*)receivePacket (sd, server);
    //         switch (ack->PacketType) {
    //             case 1: // SYN
    //                 break;
    //             case 2: // SYNACK
    //                 break;
    //             case 3: // ACK
    //                 if (ack->SeqNum == transmitted_packets[syn_ack_packet->WindowSize - packet_counter].AckNum) {
    //                     printReceived (server, client, ack);
    //                     gettimeofday (&end, NULL);
    //                     printf ("Round-trip delay = %ld ms.\n", delay(start, end));
    //                     // for(int i = 0; i < syn_ack_packet->WindowSize; i++) {
    //                     //     if (acked_packets[i]->packet.SeqNum == ack->SeqNum) {
    //                     //         printf("acked\n");
    //                     //     }
    //                     // }
    //                 } else if (ack->SeqNum == copy_packet.AckNum) {
    //                     // duplicate acks
    //                     //printf("DUPLICATE ACKS DETECTED=>");
    //                     //printReceived (server, client, ack);
    //                 }
    //                 break;
    //             case 4: // DATA
    //                 break;
    //             case 8: // EOT
    //                 break;
    //             case 9: // TO
    //                 break;
    //             default:
    //                 break;
    //         }
    //         packet_counter--;
    //     }
    // }
    
    // printf(" RTT\t\tPacketType\tSeqNum\t\tAckNum\t\tdata\t\tWindowSize\n");
    // printf("========================================================================================\n");
    // for(int i = 0; i < total_packet_count; i++) {
    //     struct Packet pkt = acked_packets[i]->packet;
    //     printf(" %ld ms\t\t%d\t\t%d\t\t%d\t\t%s\t\t%d\n", acked_packets[i]->delay, pkt.PacketType, pkt.SeqNum, pkt.AckNum, pkt.data, pkt.WindowSize);    
    // }
    
	close(sd);
	return(0);
}

