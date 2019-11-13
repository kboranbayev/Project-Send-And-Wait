// A simple UCP client program allows the user to specify a server by its domain name.

#include "handlers.h"

#define SERVER_UDP_PORT		   8000	   // Default port
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
	client.sin_port = htons(0);
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
    
    struct PacketByte *pktByte = malloc(sizeof(struct PacketByte));
    struct Packet ack_packet;
    
    // start RTT calculator
    gettimeofday(&start, NULL);
    // send SYN
    sendPacket (sd, syn_packet, server);
    printTransmitted (client, server, syn_packet);
    
    // receive SYNACK
    pktByte = receivePacket (sd, server);
    printReceived (server, client, &pktByte->packet);
    while (pktByte->packet.PacketType == 99) { // CHECK FOR TIMEOUT
        sendPacket (sd, syn_packet, server);
    }
    
    if (pktByte->packet.PacketType == 2 && pktByte->packet.AckNum == syn_packet.SeqNum + 1) {
        
        memset((char *)&ack_packet, 0, sizeof(ack_packet));
        ack_packet.PacketType = 3;
        ack_packet.SeqNum = pktByte->packet.AckNum;
        ack_packet.AckNum = generateNum();
        if (pktByte->packet.WindowSize < syn_packet.WindowSize) {
            ack_packet.WindowSize = pktByte->packet.WindowSize;
        } else {
            ack_packet.WindowSize = syn_packet.WindowSize;
        }
    }
    // send ACK
    sendPacket (sd, ack_packet, server);
    printTransmitted (client, server, ack_packet);
    
    gettimeofday (&end, NULL);
    printf ("Round-trip delay = %ld ms.\n", delay(start, end));
    // three-way handshake ends

    char *msg = "The objective of this project is to design and implement a basic Send-And-Wait protocol simulator. The protocol will be half-duplex and use sliding windows to send multiple packets between two hosts on a LAN with an \"unrealiable network\" between the two hosts. The following diagram depicts the model:\nYour Mission\n - You may use any language of your choice to implement the three components shown in the diagram above. It is strongly recommended that you use your code from the first assignment to implement the peer stations.\n - You will be designing an application layer protocol in this case on top of UDP (in keeping with the wireless channel model). The protocol should be able to handle network errors such as packet loss and duplicate packets. You will implement timeouts and ACKs to handle retransmissions due to lost packets (ARQ).";
    int packet_counter = 0, total_packet_count = 0, acked_packet_count = 0;
    int shift = 0;
    
    struct Packet packet1;
    struct Packet packet1_copy;
    struct Packet transmitted_packets[ack_packet.WindowSize];
    struct PacketRTT *acked_packets[getWindowSize(msg, sizeof(ack_packet.data))];
	// transmit data
    
//     while (total_packet_count < getWindowSize(msg, sizeof(ack_packet.data))) {
//         while (packet_counter < ack_packet.WindowSize) {
//             packet1.PacketType = 4; // 0 - EOT
//             if (packet1_copy.SeqNum == packet1.SeqNum && total_packet_count < getWindowSize(msg, sizeof(ack_packet.data))) {
//                 packet1.SeqNum += sizeof(packet1.data);
//                 packet1.AckNum += sizeof(packet1.data);
//                 packet1.WindowSize = packet1_copy.WindowSize;
//                 char *to = (char *) malloc(sizeof(packet1.data));
//                 strncpy(to, msg + shift, sizeof(to));
//                 strncpy(packet1.data, to, sizeof(packet1.data));
//             } else if (total_packet_count < getWindowSize(msg, sizeof(ack_packet.data))) {
//                 packet1.SeqNum = generateNum();
//                 packet1.AckNum = packet1.SeqNum + 100;
//                 packet1.WindowSize = ack_packet.WindowSize;
//                 strncpy(packet1.data, msg, sizeof(packet1.data));
//             } else {
//                 packet1.PacketType = 0;
//                 packet1.SeqNum += sizeof(packet1.data);
//                 packet1.AckNum += sizeof(packet1.data);
//                 packet1.WindowSize = packet1_copy.WindowSize;
//                 memset(packet1.data, 0, sizeof(packet1.data));
//                 sendPacket (sd, packet1, server);
//                 printTransmitted (client, server, packet1);
//                 break;
//             }
//             
//             gettimeofday(&start, NULL);
//             sendPacket (sd, packet1, server);
//             printTransmitted (client, server, packet1);
//             
//             transmitted_packets[packet_counter] = packet1;
//             struct PacketRTT *acked_packet = (struct PacketRTT*) malloc(sizeof(struct PacketRTT));
//             acked_packet->packet = packet1;
//             acked_packet->start = start;
//             acked_packets[acked_packet_count] = acked_packet;
//             packet1_copy = packet1;
//             packet_counter++;
//             total_packet_count++;
//             acked_packet_count++;
//             shift += sizeof(packet1.data);
//         }
//         
//         struct Packet *ack = malloc(sizeof(struct Packet));
//         while (packet_counter > 0) {
//             ack = receivePacket (sd, server);
//             switch (ack->PacketType) {
//                 case 0: // EOT
//                     break;
//                 case 1: // SYN
//                     break;
//                 case 2: // SYNACK
//                     break;
//                 case 3: // ACK
//                     if (ack->SeqNum == transmitted_packets[ack_packet.WindowSize - packet_counter].AckNum) {
//                         printReceived (server, client, ack);
//                         gettimeofday (&end, NULL);
//                         for(int i = 0; i < ack_packet.WindowSize; i++) {
//                             if (acked_packets[i]->packet.SeqNum == ack->SeqNum) {
//                                 acked_packets[i]->delay = delay(acked_packets[i]->start, end);
//                             }
//                         }
//                     }
//                     break;
//                 case 4: // DATA
//                     break;
//                 case 99: // TO
//                     break;
//                 default:
//                     break;
//             }
//             packet_counter--;
//         }
//     }
    
    printf(" RTT\t\tPacketType\tSeqNum\t\tAckNum\t\tdata\t\tWindowSize\n");
    printf("========================================================================================\n");
    for(int i = 0; i < total_packet_count; i++) {
        struct Packet pkt = acked_packets[i]->packet;
        printf(" %ld ms\t\t%d\t\t%d\t\t%d\t\t%s\t\t%d\n", acked_packets[i]->delay, pkt.PacketType, pkt.SeqNum, pkt.AckNum, pkt.data, pkt.WindowSize);    
    }
    
	close(sd);
	return(0);
}

