#include "handlers.h"

#define SERVER_UDP_PORT 	8000	// Default port
#define WIN				    10   // Default Window Size
 
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
    to.tv_sec = 3;
    to.tv_usec = 0;
    //setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
	
	struct Packet *temp = malloc(sizeof(struct Packet));
    struct Packet previous_ack_sent;
    struct Packet* received_packets;

    previous_ack_sent.SeqNum = 0;
    int skip = 0;
    size_t max_window = 0;
	while (1)
	{   
        // cant change following to receivePacket() as sendto requires sockaddr client
        unsigned int client_len = sizeof(client);
        setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));   
        if (recvfrom (sd, temp, sizeof(*temp), 0, (struct sockaddr *)&client, &client_len) < 0)
        {
            if (previous_ack_sent.SeqNum == 0) {
                printf("timeout\n");
            } else {
                *temp = previous_ack_sent;
                skip++;
            }
        }
        if (temp->SeqNum != previous_ack_sent.SeqNum) {
            printReceived(client, server, temp);
        }
        struct Packet ack;
        if (skip < 4) {
            switch (temp->PacketType)
            {
                case 1: // SYN
                    // handle SYNACK function
                    if (temp->SeqNum != previous_ack_sent.SeqNum) {
                        ack.PacketType = 2;
                        ack.SeqNum = temp->AckNum;
                        ack.AckNum = temp->SeqNum + 25;
                        max_window = setWindowSize(temp->WindowSize, WIN);
                        ack.WindowSize = max_window;
                        memset(ack.data, 0, strlen(ack.data));
                        sendPacket (sd, ack, client);
                        printTransmitted (server, client, ack);
                        received_packets = malloc(ack.WindowSize * sizeof(struct Packet));
                    } else {
                        sendPacket (sd, ack, client);
                        printReTransmitted (server, client, ack);
                    }
                    previous_ack_sent = ack;
                    break;
                case 2: // SYNACK
                    // handle ACK function
                    break;
                case 3: // ACK
                    // handle ACK function
                    break;
                case 4: // DATA
                    // handle DATA function
                    if (temp->SeqNum != previous_ack_sent.SeqNum) {
                        strcat(msg, temp->data);
                        ack.PacketType = 3;
                        ack.SeqNum = temp->AckNum;
                        ack.AckNum = temp->SeqNum + 1;
                        ack.WindowSize = max_window - temp->WindowSize - 1;
                        received_packets[ack.WindowSize] = ack;
                    } else {
                        sendPacket (sd, ack, client);
                        printReTransmitted (server, client, ack);
                    }
                    previous_ack_sent = ack;
                    break;
                case 8: // EOT
                    // handle EOT function
                    printf("EOT received\n");
                    printf("%s\n", msg);
                    exit(1);
                    break;
                default:
                    break;
            }
            if (temp->WindowSize == 0) {
                printf("server starts sending ack from here, %d\n", max_window);
                for (size_t i = 0; i<max_window; i++) {
                    sendPacket (sd, received_packets[i], client);
                    printTransmitted (server, client, received_packets[i]);
                }
            }
            
        }
        memset(temp, 0, sizeof(*temp));
	}
	
	close(sd);
	return(0);
}
