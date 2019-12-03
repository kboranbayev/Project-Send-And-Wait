/**
 * 
 * @author Kuanysh Boranbayev
 * @author Parm Dhaliwal
 * @date November 27, 2019
 */
#include "handlers.h"

#define CLIENT_UDP_PORT        6000    // Default client port
#define SERVER_UDP_PORT		   7000	   // Default server port

int main (int argc, char **argv)
{
	int port = SERVER_UDP_PORT;
	int sd;
	char *pname, *host, *filename;
	struct	hostent	*hp;
	struct	sockaddr_in server, client;
	struct  timeval start, end;
    
	pname = argv[0];
	argc--;
	argv++;
	if (argc > 0 && (strcmp(*argv, "-f") == 0))
	{
		if (--argc > 0)
		{
			argc--;
			argv++;
            filename = *argv;
            argv++;
		}
		else
		{
			fprintf (stderr,"Usage: %s [-f send.txt] host [port]\n", pname);
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

    printf("filename %s\n", filename);
    printf("host %s\n", host);

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
    
    

    FILE *fp;

    if ((fp = fopen(filename, "r")) == NULL) {
       DieWithError ("Error! opening file");
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *msg = (char*)calloc(fsize, sizeof(char));
    fread(msg, sizeof(char), fsize, fp);
    
    struct Packet syn_packet;
    
    syn_packet.PacketType = 1;
    syn_packet.SeqNum = 100;
    syn_packet.AckNum = 200;
    
    syn_packet.WindowSize = getTotalPacketCount(msg, sizeof(syn_packet.data));

    memset(syn_packet.data, 0, sizeof(syn_packet.data));
    
    struct Packet *syn_ack_packet = malloc(sizeof(struct Packet));
    
    int n, server_len = sizeof(server);
    
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    // start RTT calculator
    gettimeofday(&start, NULL);
    
    

    // 2 way handshake session begins
    while (1) {
        // send SYN
        sendPacket (sd, syn_packet, server);
        printTransmitted (client, server, syn_packet);
        // receive SYNACK
        while ((n = recvfrom (sd, syn_ack_packet, sizeof(*syn_ack_packet), 0, (struct sockaddr *)&server, (unsigned int *)&server_len)) < 0) {
            syn_packet.re = 1;
            sendPacket (sd, syn_packet, server);
            printReTransmitted( client, server, syn_packet);
        }
        break;
    }
    
    gettimeofday (&end, NULL);
    printReceivedRTT (server, client, syn_ack_packet, delay(start, end));
    
    // 2 way handshake session ends
    
    int packet_counter = 0, total_packet_count = 0, windowSize_counter = 1;
    int shift = 0;
    if (delay(start, end) != 0) {
        timeout.tv_sec = TIMEOUT * delay(start, end);
        timeout.tv_usec = 0;
    }
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    struct Packet copy_packet;
    copy_packet.SeqNum = 0;
    struct PacketSent transmitted_packets[syn_ack_packet->WindowSize];
    struct Packet *transmit_packets = malloc(syn_ack_packet->WindowSize * sizeof(struct Packet));;
    struct PacketRTT *acked_packets = malloc(syn_ack_packet->WindowSize * sizeof(struct PacketRTT));;

	// transmit data
    int retransmission_needed = 0;
    while (total_packet_count <= getTotalPacketCount(msg, sizeof(syn_ack_packet->data)) + 1) {
        
        // transmit section
        if (retransmission_needed != 1) {
            for (int i = 0; i < syn_ack_packet->WindowSize; i++) {
                struct Packet data_packet;
                data_packet.PacketType = 4;
                if (copy_packet.SeqNum == 0 && total_packet_count <= getTotalPacketCount(msg, sizeof(syn_ack_packet->data))) { // first packet
                    data_packet.last = 0;
                    data_packet.SeqNum = syn_ack_packet->AckNum;
                    data_packet.AckNum = syn_ack_packet->WindowSize;
                    strncpy(data_packet.data, msg, sizeof(data_packet.data));
                } else if (copy_packet.SeqNum != 0 && total_packet_count < getTotalPacketCount(msg, sizeof(syn_ack_packet->data))) {
                    if (total_packet_count < getTotalPacketCount(msg, sizeof(syn_ack_packet->data))) {
                        data_packet.SeqNum = copy_packet.SeqNum + syn_ack_packet->WindowSize;
                        data_packet.AckNum = copy_packet.AckNum + syn_ack_packet->WindowSize;
                        char *to = (char *) malloc(sizeof(data_packet.data));
                        strncpy(to, msg + shift, sizeof(data_packet.data));
                        strncpy(data_packet.data, to, sizeof(data_packet.data));
                    } else {
                        puts("no more data to send EOT here");
                    }
                } else if (total_packet_count == getTotalPacketCount(msg, sizeof(syn_ack_packet->data)) + 1) { // Last packet
                    data_packet.PacketType = 8;
                    data_packet.SeqNum = copy_packet.SeqNum + syn_ack_packet->WindowSize;
                    data_packet.AckNum = 0;
                    data_packet.last = 1;
                    memset(data_packet.data, 0, sizeof(data_packet.data));
                }
                
                if (windowSize_counter == syn_ack_packet->WindowSize) {
                    windowSize_counter = 0;
                }
                data_packet.WindowSize = i + 1;
                if (strcmp(data_packet.data, copy_packet.data) == 0) {
                    memset(data_packet.data, 0, sizeof(data_packet.data));
                }
                if (data_packet.PacketType == 8) {
                    data_packet.WindowSize = syn_ack_packet->WindowSize;
                }
                transmit_packets[i] = data_packet;
                copy_packet = data_packet;
                windowSize_counter++;
                packet_counter++;
                shift += sizeof(data_packet.data);
                total_packet_count++;
                if (data_packet.PacketType == 8) {
                    break;
                }
            }
            for (int i = 0; i < syn_ack_packet->WindowSize; i++) {
                transmit_packets[i].last = 0;
                if (i == syn_ack_packet->WindowSize - 1 || transmit_packets[i].PacketType == 8) {
                    transmit_packets[i].last = 1;
                    transmitted_packets[i].packet = transmit_packets[i];
                    transmitted_packets[i].acked = 0;
                    sendPacket (sd,  transmit_packets[i], server);
                    printTransmitted (client, server,  transmit_packets[i]);
                    break;
                }
                transmit_packets[i].re = 0;
                transmitted_packets[i].packet = transmit_packets[i];
                transmitted_packets[i].acked = 0;
                sendPacket (sd,  transmit_packets[i], server);
                printTransmitted (client, server,  transmit_packets[i]);
            }
        } else {
            for (int i = 0; i < syn_ack_packet->WindowSize; i++) {
                if (i == syn_ack_packet->WindowSize - 1) {
                    transmit_packets[i].last = 1;
                }
                transmit_packets[i].re = 1;
                transmitted_packets[i].packet = transmit_packets[i];
                transmitted_packets[i].acked = 0;
                sendPacket (sd,  transmit_packets[i], server);
                printReTransmitted (client, server,  transmit_packets[i]);
            }
            retransmission_needed = 0;
        }

        // receiving section
        int received_ack_count = 0;
        struct Packet *ack = malloc(sizeof(struct Packet));
        //struct Packet test;
        while (1) {
            //ack = (struct Packet*)receivePacket (sd, server);
            timeout.tv_sec = TIMEOUT / 2;
            setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            if ((n = recvfrom (sd, ack, sizeof(*ack), 0, (struct sockaddr *)&server, (unsigned int *)&server_len)) < 0) {
                puts("TIMEOUT");
                retransmission_needed = 1;
                break;
            }
            printf("SIZE = %d\n", n);
            switch (ack->PacketType) {
                case 1: // SYN
                    break;
                case 2: // SYNACK
                    break;
                case 3: // ACK
                    if (ack->SeqNum == transmitted_packets[received_ack_count].packet.AckNum) {
                        printReceivedRTT (server, client, ack, delay(start, end));
                        gettimeofday (&end, NULL);
                        transmitted_packets[received_ack_count].acked = 1;
                        acked_packets[received_ack_count].packet = *ack;
                        acked_packets[received_ack_count].delay = delay(start, end);
                        received_ack_count++;
                    } else if (ack->SeqNum != transmitted_packets[received_ack_count].packet.AckNum) {
                        gettimeofday (&end, NULL);
                        for (int i = 0; i < syn_ack_packet->WindowSize; i++) {
                            if (ack->SeqNum == transmitted_packets[i].packet.AckNum) {
                                transmitted_packets[i].acked = 1;
                            }
                        }
                        printReceivedRTT (server, client, ack, delay(start, end));
                    } else if (ack->SeqNum == transmitted_packets[received_ack_count].packet.SeqNum) {
                        // duplicate acks
                        gettimeofday (&end, NULL);
                        puts("DUPLICATE");
                        printReceivedRTT (server, client, ack, delay(start, end));
                        
                    }
                    //test = *ack;
                    break;
                case 4: // DATA
                    break;
                case 5: // NAK
                    printReceived (server, client, ack);
                    retransmission_needed = 1;
                    break;
                case 8: // EOT
                    break;
                case 9: // TO
                    puts("TIMEOUT");
                    retransmission_needed = 1;
                    break;
                default:
                    break;
            }
            if (retransmission_needed == 1) {
                break;
            } else if (ack->last == 1) {
                for (int i = 0; i < syn_ack_packet->WindowSize; i++) {
                    if (transmitted_packets[i].acked != 1) {
                        transmitted_packets[i].packet.re = 1;
                        transmitted_packets[i].acked = 1;
                        sendPacket (sd, transmitted_packets[i].packet, server);
                        printReTransmitted (client, server, transmitted_packets[i].packet);
                    }
                    retransmission_needed = 0;
                }
                break;
            }
        }
    }
    
	close(sd);
    fclose(fp);
	return(0);
}