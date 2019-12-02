/**
 * 
 * @author Kuanysh Boranbayev
 * @author Parm Dhaliwal
 * @date November 27, 2019
 */

#include "handlers.h"

#define SERVER_UDP_PORT 	8000	// Default server port
#define WIN				    15      // Default Window Size
 
long int window_size = WIN;

int main (int argc, char **argv)
{
	int	sd, port;
	struct	sockaddr_in server, client;
    //struct  timeval start, end;
    char *msg;
    
	switch(argc)
	{
		case 1:
			port = SERVER_UDP_PORT;	// Default port
		    break;
		case 2:
            if (argv[1] && atoi(argv[1]) != 0 && window_size != atoi(argv[1])) {
                //window_size = (int)atol(argv[1]); // GCC has issues here
            }
		    break;
		default:
			fprintf(stderr, "Usage: %s [window size]\n", argv[0]);
			exit(1);
   	}

	// Create a datagram socket
	if ((sd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		DieWithError ("Can't create a socket");
	}
    
	// Bind an address to the socket
    memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET; 
	server.sin_port = htons(port); 
	server.sin_addr.s_addr = htonl(INADDR_ANY);
    
	if (bind (sd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		DieWithError ("Can't bind name to socket");
	}
	
    struct timeval to;
    to.tv_sec = TIMEOUT / 2;
    to.tv_usec = 0;
	
	struct Packet *temp = malloc(sizeof(struct Packet));
    struct Packet previous_ack_sent;
    struct Packet *received_packets, *transmit_packets, *all_packets;

    previous_ack_sent.SeqNum = 0;
    int skip = 0, packet_counter = 0;
    long int max_window = 0, max_packets = 0;
    int retransmit = 0, all_data_received = 0, packet_in_flight = 0, nak_count = 0, write_count = 0;
    
    
	while (1)
	{
        // cant change following to receivePacket() as sendto requires sockaddr client
        while (1) {
    
            setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
            unsigned int client_len = sizeof(client);
            
            if (recvfrom (sd, temp, sizeof(*temp), 0, (struct sockaddr *)&client, &client_len) < 0)
            {
                if (previous_ack_sent.SeqNum == 0) {
                    puts("TIMEOUT");
                } else if (previous_ack_sent.SeqNum != 0 && previous_ack_sent.PacketType == 2) {
                    puts("TIMEOUT");
                    *temp = previous_ack_sent;
                } else {
                    puts("TIMEOUT");
                    retransmit = 1;
                }
            }
            if (temp->SeqNum != previous_ack_sent.SeqNum) {
                
            }
            struct Packet ack;
            if (skip < 4 && retransmit == 0) {
                switch (temp->PacketType)
                {
                    case 1: // SYN
                        // handle SYNACK function
                        printReceived(client, server, temp);
                        max_window = setMaxWindowSize (temp->WindowSize, window_size);
                        max_packets = setMaxPacketCount (temp->WindowSize, window_size);
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
                            
                            all_packets = malloc((max_packets+1) * sizeof(struct Packet));
                            msg = (char*)calloc(max_packets*max_packets, sizeof(char));
                            for (int i = 0; i < max_packets; i++) {
                                all_packets[i].SeqNum = 0;
                            }
                        } else {
                            ack.re = 1;
                            //sendPacket (sd, ack, client);
                            //printReTransmitted (server, client, ack);
                        }
                        previous_ack_sent = ack;
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
                        // handle ACK function
                        break;
                    case 4: // DATA
                        // handle DATA function
                        if (packet_in_flight == temp->WindowSize - 1 || temp->re == 1) {
                            printReceived(client, server, temp);
                            ack.PacketType = 3;
                            ack.SeqNum = temp->AckNum;
                            ack.AckNum = temp->SeqNum + (max_window * max_window);
                            ack.WindowSize = temp->WindowSize;
                            int ignore = 0;
                            for (int i = 0; i < max_window; i++) {
                                if (received_packets[i].SeqNum == temp->SeqNum) {
                                    puts("DUPLICATE");
                                    ignore = 1;
                                    break;
                                }
                            }
                            for (int i = 0; i < max_packets; i++) {
                                if (all_packets[i].SeqNum == temp->SeqNum) {
                                    break;
                                }
                            }
                            if (ignore == 0) {
                                received_packets[temp->WindowSize - 1] = *temp;
                                transmit_packets[temp->WindowSize - 1] = ack;
         
                                packet_counter++;
                            }              
                            if (packet_in_flight == max_window) {
                                packet_in_flight = 0;
                            } else {
                                packet_in_flight++;
                            }
                            
                            previous_ack_sent = ack;
                        } else if (packet_in_flight != temp->WindowSize - 1) {
                            puts("OUT OF ORDER");
                            printReceived(client, server, temp);
                            ack.PacketType = 3;
                            ack.SeqNum = temp->AckNum;
                            ack.AckNum = temp->SeqNum + (max_window * max_window);
                            ack.WindowSize = temp->WindowSize;
                            int ignore = 0;
                            for (int i = 0; i < max_window; i++) {
                                if (received_packets[i].SeqNum == temp->SeqNum) {
                                    puts("DUPLICATE");
                                    ignore = 1;
                                    break;
                                }
                            }
                            for (int i = 0; i < max_packets; i++) {
                                if (all_packets[i].SeqNum == temp->SeqNum) {
                                    break;
                                }
                            }
                            if (ignore == 0) {
                                received_packets[temp->WindowSize - 1] = *temp;
                                transmit_packets[temp->WindowSize - 1] = ack;

                                packet_counter++;
                            }
                            
                            if (packet_in_flight == max_window) {
                                packet_in_flight = 0;
                            } else {
                                packet_in_flight++;
                            }
                            
                            previous_ack_sent = ack;
                        }
                        
                        if (temp->WindowSize % max_window == 0) {
                            for (int i = 0; i < max_window; i++) {
                                if (received_packets[i].WindowSize != (i + 1)) {
                                    all_data_received = 0;
                                    break;
                                } else {
                                    all_data_received = 1;
                                }
                            }
                        }

                        break;
                    case 8: // EOT
                        // handle EOT function
                        printReceived(client, server, temp);
                        puts("EOT received");
                        break;
                    default:
                        break;
                }
                if (nak_count > 3) {
                    puts("WARNING => Too many unordered packets, sending NAK");
                    struct Packet nak;
                    nak.PacketType = 5;
                    sendPacket (sd, nak, client);
                    printTransmitted (server, client, nak);
                    nak_count = 0;
                } else if ((temp->WindowSize == max_window && temp->PacketType == 4) || (temp->last == 1 && temp->WindowSize % max_window == 0) || temp->PacketType == 8) {
                    for (int i = 0; i < max_window; i++) {
                        if (transmit_packets[i].WindowSize == i + 1) {
                            if (i == max_window - 1) {
                                transmit_packets[i].last = 1;
                            }
                            sendPacket (sd, transmit_packets[i], client);
                            printTransmitted (server, client, transmit_packets[i]);
                        }
                    }
                }
            } else if (retransmit == 1) {
                for (int i = 0; i < max_window; i++) {
                    if (transmit_packets[i].WindowSize == i + 1) {
                        transmit_packets[i].re = 1;
                        sendPacket (sd, transmit_packets[i], client);
                        printReTransmitted (server, client, transmit_packets[i]);
                    }
                }
                retransmit = 0;
            }
            if (all_data_received == 1 ||  temp->PacketType == 8) {
                int write_to_file = 1;
                for (int i = 0; i < write_count; i++) {
                    if (i < max_packets) {
                        for (int j = 0; j < max_window; j++) {
                            if (all_packets[i].SeqNum == received_packets[j].SeqNum) {
                                memset(received_packets[j].data, 0, sizeof(*received_packets[j].data));
                                break;
                            }
                        }
                    } else {
                        break;
                    }
                }
                if (write_to_file == 1) {
                    for (int i = 0; i < max_window; i++) {
                        if (strlen(received_packets[i].data) == PAYLOAD_LEN + 1) {
                            for (int j = 0; j < PAYLOAD_LEN; j++) {
                                strncat(msg, &received_packets[i].data[j], 1);
                            }
                        } else if ((strlen(received_packets[i].data) <= PAYLOAD_LEN)) {
                            strcat(msg, received_packets[i].data);
                        } else {
                            break;
                        }
                        all_packets[write_count] = received_packets[i];
                        write_count++;
                        
                        memset(received_packets[i].data, 0, sizeof(*received_packets[i].data));
                        memset(received_packets, 0, sizeof(struct Packet ));
                        memset(transmit_packets, 0, sizeof(struct Packet ));
                    }
                    FILE *fp;

                    if ((fp = fopen("data/received.txt", "w")) == NULL) {
                        DieWithError ("Error! opening file");
                    }

                    fprintf(fp, "%s", msg);

                    fclose(fp);
                    
                    //memset(received_packets, 0, sizeof(&received_packets));
                    all_data_received = 0;
                }
            }
            if (temp->PacketType == 8) {
                puts("DATA TRANSMISSION => FIN");
                exit(1);
            }

            break;
            to.tv_sec = TIMEOUT;
            memset(temp, 0, sizeof(*temp));
        }
	}
	
	close(sd);
	return(0);
}