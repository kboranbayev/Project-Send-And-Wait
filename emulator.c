#include "handlers.h"

#define SERVER_UDP_PORT		8000	// Default port
#define MAXLEN				65000    // Maximum Buffer length
#define DEFLEN				64	// Default Length

int main (int argc, char **argv)
{
	int	sd1, sd2, receiver_len, sender_len, port, n;
	char	buf[MAXLEN];
	struct	sockaddr_in receiver, sender, emulator;
    struct hostent *hp;
    char *host;
    
	switch(argc)
	{
		case 1:
            port = SERVER_UDP_PORT;
		break;
		case 2:
            port = atoi(argv[1]);
		break;
		default:
			exit(1);
   	}

	// Create a datagram socket
	if ((sd1 = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		DieWithError ("Can't create a socket");
	}
	
	// Create a datagram socket
	if ((sd2 = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		DieWithError ("Can't create a socket");
	}

	// Bind an address to the socket
	bzero((char *)&emulator, sizeof(emulator));
	emulator.sin_family = AF_INET; 
	emulator.sin_port = htons(port); 
	emulator.sin_addr.s_addr = htonl(INADDR_ANY);
    
    bzero((char *)&receiver, sizeof(receiver));
	receiver.sin_family = AF_INET; 
	receiver.sin_port = htons(8000);
    
    
    if ((hp = gethostbyname("127.0.0.1")) == NULL)
    {
        DieWithError ("Can't get server's IP address");
    }
    bcopy(hp->h_addr, (char *)&receiver.sin_addr, hp->h_length);
    
	if (bind (sd1, (struct sockaddr *)&emulator, sizeof(emulator)) == -1)
	{
		DieWithError ("Can't bind name to socket");
	}
	
	while (1)
	{
        struct Packet *temp = malloc(sizeof(struct Packet));
		sender_len = sizeof(sender);
		if ((n = recvfrom (sd1, temp, sizeof(*temp), 0, (struct sockaddr *)&sender, &sender_len)) < 0)
		{
			DieWithError ("recvfrom error");
		}
        printReceived (sender, emulator, temp);
        
        struct Packet tmp;
        tmp.PacketType = temp->PacketType;
        tmp.SeqNum = temp->SeqNum;
        tmp.AckNum = temp->AckNum;
        strncpy(tmp.data, temp->data, sizeof(tmp.data));
        tmp.WindowSize = temp->WindowSize;
        
        receiver_len = sizeof(receiver);
        if (sendto (sd2, (struct Packet *)&tmp, sizeof(tmp), 0, (struct sockaddr *)&receiver, receiver_len) == -1)
        {
            DieWithError ("sendto failure");
        }
        printTransmitted (emulator, receiver, tmp);
        
        struct Packet *ack = malloc(sizeof(struct Packet));
        if ((n = recvfrom (sd2, ack, sizeof(*ack), 0, (struct sockaddr *)&receiver, &receiver_len)) < 0)
		{
			DieWithError ("recvfrom error");
		}
        printReceived (receiver, emulator, ack);
        
        struct Packet tmp1;
        tmp1.PacketType = ack->PacketType;
        tmp1.SeqNum = ack->SeqNum;
        tmp1.AckNum = ack->AckNum;
        strncpy(tmp1.data, ack->data, sizeof(tmp1.data));
        tmp1.WindowSize = ack->WindowSize;
        
        if (sendto (sd1, (struct Packet *)&tmp1, sizeof(tmp1), 0, (struct sockaddr *)&sender, sender_len) == -1)
        {
            DieWithError ("sendto failure");
        }
        printTransmitted (emulator, sender, tmp1);
        
	}
	close(sd1);
    close(sd2);
	return(0);
}
