#include "handlers.h"

#define CLIENT_UDP_PORT		7000	// Default port
#define SERVER_UDP_PORT		8000	// Default port

int main (int argc, char **argv)
{
	int	sd1, sd2, receiver_len, sender_len, port_server = SERVER_UDP_PORT, port_client = CLIENT_UDP_PORT, n;
	struct	sockaddr_in receiver, sender, emulator;
    struct hostent *hp;
    char *pname, *host;
    
    pname = argv[0];
    argc--;
    argv++;
    if (argc > 0 && (strcmp(*argv, "-n") == 0))
	{
		if (--argc > 0)
		{
			argc--;
			argv++;
		}
		else
		{
			fprintf (stderr,"Usage: %s [-n noise] host [port]\n", pname);
			exit(1);
		}
	}
	if (argc > 0) 
	{
		host = *argv;
			if (--argc > 0)
				port_server = atoi(*++argv);
	}
	else
	{
		fprintf(stderr, "Usage:	%s [-n noise] host [port]\n", pname);
		exit(1);
	}
    
/*    
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
   	}*/

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
	emulator.sin_port = htons(port_client); 
	emulator.sin_addr.s_addr = htonl(INADDR_ANY);
    
    bzero((char *)&receiver, sizeof(receiver));
	receiver.sin_family = AF_INET; 
	receiver.sin_port = htons(port_server);
    
    
    if ((hp = gethostbyname(host)) == NULL)
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
        
        if (temp->PacketType == 1) {
            memset((char *)&tmp, 0, sizeof(tmp));
            tmp = *temp;
            
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
            
            struct Packet tmp1 = *ack;
            
            if (sendto (sd1, (struct Packet *)&tmp1, sizeof(tmp1), 0, (struct sockaddr *)&sender, sender_len) == -1)
            {
                DieWithError ("sendto failure");
            }
            printTransmitted (emulator, sender, tmp1);
        } else if (temp->PacketType == 3) {
            memset((char *)&tmp, 0, sizeof(tmp));
            tmp = *temp;
            receiver_len = sizeof(receiver);
            if (sendto (sd2, (struct Packet *)&tmp, sizeof(tmp), 0, (struct sockaddr *)&receiver, receiver_len) == -1)
            {
                DieWithError ("sendto failure");
            }
            printTransmitted (emulator, receiver, tmp);
        
        } else {
            memset((char *)&tmp, 0, sizeof(tmp));
            tmp = *temp;
            
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
            
            struct Packet tmp1 = *ack;
            
            if (sendto (sd1, (struct Packet *)&tmp1, sizeof(tmp1), 0, (struct sockaddr *)&sender, sender_len) == -1)
            {
                DieWithError ("sendto failure");
            }
            printTransmitted (emulator, sender, tmp1);
        }
	}
	close(sd1);
    close(sd2);
	return(0);
}
