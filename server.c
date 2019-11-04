#include "handlers.h"

#define SERVER_UDP_PORT 	8000	// Default port
#define MAXLEN			    65000	//Buffer length
 
int main (int argc, char **argv)
{
	int	sd, client_len, port, n;
	char	buf[MAXLEN];
	struct	sockaddr_in server, client;
    char msg[MAXLEN];
    
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

	while (1)
	{
        struct Packet *temp = malloc(sizeof(struct Packet));
		client_len = sizeof(client);
		if ((n = recvfrom (sd, temp, sizeof(*temp), 0, (struct sockaddr *)&client, &client_len)) < 0)
		{
			DieWithError ("recvfrom error");
		}
		
		printReceived(client, server, temp);
		
        struct Packet ack;
        switch (temp->PacketType)
        {
            case 0: // EOT
                // handle EOT function
                printf("EOT received\n");
                printf("%s\n", msg);
                break;
            case 1: // ACK
                // handle ACK function
                break;
            case 2: // DATA
                // handle DATA function
                strcat(msg, temp->data);
                ack.PacketType = 1;
                ack.SeqNum = temp->AckNum;
                ack.AckNum = temp->AckNum + 1;
                ack.WindowSize = temp->WindowSize;
                if (sendto (sd, (struct Packet *)&ack, sizeof(ack), 0,(struct sockaddr *)&client, client_len) != n)
                {
                    DieWithError ("sendto error");
                }
                printTransmitted (server, client, ack);
                break;
            default:
                exit(1);
        }
	}
	
	close(sd);
	return(0);
}
