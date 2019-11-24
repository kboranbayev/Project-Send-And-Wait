#include "handlers.h"

#define SERVER_UDP_PORT 	8000	// Default port
#define WIN				    22   // Default Window Size
 
int main (int argc, char **argv)
{
	int	sd, port;
	struct	sockaddr_in server, client;
    char msg[1024];
    
	switch(argc)
	{
		case 1:
			port = SERVER_UDP_PORT;	// Default port
		break;
		case 2:
			port = atoi(argv[1]);	//User specified port
		break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
   	}

	// Create a datagram socket
	if ((sd = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		DieWithError ("Can't create a socket");
	}

	// Bind an address to the socket
    memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET; 
	server.sin_port = htons(port); 
	server.sin_addr.s_addr = htonl(INADDR_ANY);
    
	if (bind (sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		DieWithError ("Can't bind name to socket");
	}
	
    struct timeval to;
    to.tv_sec = 2;
    to.tv_usec = 0;
	
	struct Packet *temp = malloc(sizeof(struct Packet));
    struct Packet previous_ack_sent;
    struct Packet *received_packets, *transmit_packets;

    previous_ack_sent.SeqNum = 0;
    int skip = 0, packet_counter = 0;
    int max_window = 0, max_packets = 0;

    int all_data_received = 0;
    memset(msg, 0, sizeof(msg));
	while (1)
	{
        // cant change following to receivePacket() as sendto requires sockaddr client
        while (1) {
            unsigned int client_len = sizeof(client);
            setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
            if (recvfrom (sd, temp, sizeof(*temp), 0, (struct sockaddr *)&client, &client_len) < 0)
            {
                if (previous_ack_sent.SeqNum == 0) {
                    printf("timeout\n");
                } else if (previous_ack_sent.SeqNum != 0 && previous_ack_sent.PacketType != 3) {
                    printf("tout\n");
                    *temp = previous_ack_sent;
                } else {
                    //skip++;
                }
            }
            if (temp->SeqNum != previous_ack_sent.SeqNum) {
                
            }
            struct Packet ack;
            if (skip < 4) {
                switch (temp->PacketType)
                {
                    case 1: // SYN
                        // handle SYNACK function
                        printReceived(client, server, temp);
                        max_window = setMaxWindowSize (temp->WindowSize, WIN);
                        max_packets = setMaxPacketCount (temp->WindowSize, WIN);
                        if (temp->SeqNum != previous_ack_sent.SeqNum) {
                            ack.PacketType = 2;
                            ack.SeqNum = temp->AckNum;
                            ack.AckNum = temp->SeqNum + max_window;
                            ack.WindowSize = max_window;
                            memset(ack.data, 0, strlen(ack.data));
                            sendPacket (sd, ack, client);
                            printTransmitted (server, client, ack);
                            received_packets = malloc(ack.WindowSize * sizeof(struct Packet));
                            transmit_packets = malloc(ack.WindowSize * sizeof(struct Packet));
                            previous_ack_sent = ack;
                        } else {
                            ack.re = 1;
                            sendPacket (sd, ack, client);
                            printReTransmitted (server, client, ack);
                        }
                        break;
                    case 2: // SYNACK
                        if (temp->SeqNum == previous_ack_sent.SeqNum) {
                            previous_ack_sent.re = 1;
                            sendPacket (sd, previous_ack_sent, client);
                            printReTransmitted (server, client, previous_ack_sent);
                        }
                        // handle ACK function
                        break;
                    case 3: // ACK
                        if (temp->SeqNum == previous_ack_sent.SeqNum) {
                            sendPacket (sd, previous_ack_sent, client);
                            printReTransmitted (server, client, previous_ack_sent);
                        }
                        if (temp->WindowSize == 0) {
                            printf("server starts sending ack from here, %d\n", max_window);
                        }
                        // handle ACK function
                        break;
                    case 4: // DATA
                        // handle DATA function  
                        if (temp->SeqNum != previous_ack_sent.SeqNum) {
                            if (temp->re == 1) {
                                printReceivedDuplicate(client, server, temp);
                                printf("checking and fixing previous packet sequence\n");
                                for (int i = 0; i < max_window; i++) {
                                    if (received_packets[i].SeqNum == temp->SeqNum) {
                                        ack.PacketType = 3;
                                        ack.SeqNum = temp->AckNum;
                                        ack.AckNum = temp->SeqNum + (max_window * max_window);
                                        ack.WindowSize = temp->WindowSize;
                                        //ack.re == 1;
                                        printf("found missing packet, fixed the flow\n");
                                        break;
                                    }
                                }
                                printf("no missing packets\n");
                                break;
                            }
                            printReceived(client, server, temp);
                            int ignore = 0;
                            for (int i = 0; i < max_window; i++) {
                                if (received_packets[i].SeqNum == temp->SeqNum) {
                                    ignore = 1;
                                    break;
                                }
                            }
                            if (ignore != 1 && packet_counter <= max_packets + 1) {
                                ack.PacketType = 3;
                                ack.SeqNum = temp->AckNum;
                                ack.AckNum = temp->SeqNum + (max_window * max_window);
                                ack.WindowSize = temp->WindowSize;
                                received_packets[temp->WindowSize - 1] = *temp;
                                transmit_packets[temp->WindowSize - 1] = ack;
                                packet_counter++;
                            } else {
                                printf("ignore triggered, max_packet %d, packet_counter = %d\n", max_packets, packet_counter);
                            }
                        }
                        if (packet_counter % max_window == 0) {
                            for (int i = 0; i < max_window; i++) {
                                if (received_packets[i].WindowSize != (i + 1)) {
                                    all_data_received = 0;
                                } else {
                                    all_data_received = 1;
                                }
                            }
                        }
                        previous_ack_sent = ack;
                        break;
                    case 8: // EOT
                        // handle EOT function
                        printReceived(client, server, temp);
                        printf("EOT received\n");
                        break;
                    default:
                        break;
                }
                if ((temp->last == 1 && packet_counter % max_window == 0) || temp->PacketType == 8) {
                    for (int i = 0; i<max_window; i++) {
                        if (i == max_window - 1) {
                            transmit_packets[i].last = 1;
                        }
                        sendPacket (sd, transmit_packets[i], client);
                        printTransmitted (server, client, transmit_packets[i]);
                    }
                }
            }
            if (all_data_received == 1 ||  temp->PacketType == 8) {
                for (int i = 0; i < max_window; i++) {
                    if (strlen(received_packets[i].data) == 6) {
                        for (int j = 0; j < 5; j++) {
                            strncat(msg, &received_packets[i].data[j], 1);
                        }
                        
                    } else {
                        strcat(msg, received_packets[i].data);
                    }
                    memset(received_packets[i].data, 0, sizeof(*received_packets[i].data));
                    memset(received_packets, 0, sizeof(struct Packet ));
                    memset(transmit_packets, 0, sizeof(struct Packet ));
                }
                printf("%s\n", msg);
                FILE *fp;

                if ((fp = fopen("data/received.txt", "w")) == NULL) {
                    // Program exits if the file pointer returns NULL.
                    DieWithError ("Error! opening file");
                }

                fprintf(fp, "%s", msg);
                memset(temp, 0, sizeof(*temp));
                memset(received_packets, 0, sizeof(*received_packets));
                all_data_received = 0;
                break;
            }
            memset(temp, 0, sizeof(*temp));
        }
	}
	
	close(sd);
	return(0);
}
