#include "handlers.h"

#define SERVER_UDP_PORT 	8000	// Default port
#define WIN				    1460   // Default Window Size
 
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
	
	
	struct Packet *temp = malloc(sizeof(struct Packet));
	while (1)
	{
        // cant change following to receivePacket() as sendto requires sockaddr client
        unsigned int client_len = sizeof(client);
        if (recvfrom (sd, temp, sizeof(*temp), 0, (struct sockaddr *)&client, &client_len) < 0)
        {
            perror("TIMEOUT");
            temp->PacketType = 99;
        }
		printReceived(client, server, temp);
		
        struct Packet ack;
        switch (temp->PacketType)
        {
            case 0: // EOT
                // handle EOT function
                printf("EOT received\n");
                printf("%s\n", msg);
                exit(1);
                break;
            case 1: // SYN
                // handle SYNACK function
                ack.PacketType = 2;
                ack.SeqNum = 200;
                ack.AckNum = temp->SeqNum + 1;
                ack.WindowSize = 5;
               
                sendPacket (sd, ack, client);
                printTransmitted (server, client, ack);
                break;
            case 2: // SYNACK
                // handle ACK function
                ack.PacketType = 3;
                ack.SeqNum = temp->AckNum;
                ack.AckNum = temp->SeqNum + 1;
                ack.WindowSize = temp->WindowSize;
                
                sendPacket (sd, ack, client);
                printTransmitted (server, client, ack);
                break;
            case 3: // ACK
                // handle ACK function
                break;
            case 4: // DATA
                // handle DATA function
                strcat(msg, temp->data);
                ack.PacketType = 3;
                ack.SeqNum = temp->AckNum;
                ack.AckNum = temp->SeqNum + 1;
                ack.WindowSize = temp->WindowSize;
                
                sendPacket (sd, ack, client);
                printTransmitted (server, client, ack);
                break;
            case 99: // TIMEOUT
                // handle TIMEOUT
                sendPacket (sd, ack, client);
                printTransmitted (server, client, ack);
                break;
            default:
                break;
        }
        memset(temp, 0, sizeof(*temp));
	}
	
	close(sd);
	return(0);
}
