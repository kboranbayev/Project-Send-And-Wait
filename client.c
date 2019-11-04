// A simple UCP client program allows the user to specify a server by its domain name.

#include "handlers.h"

#define SERVER_UDP_PORT		8000	// Default port
#define MAXLEN				65000   // Maximum Buffer length
#define DEFLEN				64	    // Default Length

int main (int argc, char **argv)
{
	int port = SERVER_UDP_PORT;
	int sd, server_len;
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
	
    
    char *msg = "Hello, this is my first attempt at sending a data via udp. Wish me luck, you bastards! All hail, the King!";
    
    struct Packet packet1;
    
    packet1.PacketType = 2;
    packet1.SeqNum = generateSeqNum();
    packet1.AckNum = generateSeqNum() + 24;
    
    packet1.WindowSize = getWindowSize(msg, sizeof(packet1.data));
    
    strncpy(packet1.data, msg, sizeof(packet1.data));
    
	// transmit data 
	server_len = sizeof(server);
    
    int packet_counter = 0;
    int shift = 0;
    
    while (packet_counter < packet1.WindowSize) 
    {
        gettimeofday(&start, NULL); // start delay measure
        if (sendto (sd, (struct Packet *)&packet1, sizeof(packet1), 0, (struct sockaddr *)&server, server_len) == -1)
        {
            DieWithError ("sendto failure");
        }
        
        printTransmitted (client, server, packet1);
        packet_counter++;
        shift += sizeof(packet1.data);
        
        struct Packet *ack = malloc(sizeof(struct Packet));
        // receive ack
        if (recvfrom (sd, ack, sizeof(*ack), 0, (struct sockaddr *)&server, &server_len) < 0)
        {
            DieWithError (" recvfrom error");
        }
        printReceived (server, client, ack);
        gettimeofday (&end, NULL); // end delay measure
        printf ("Round-trip delay = %ld ms.\n", delay(start, end));
        
        if (ack->PacketType == 1) {
            if (packet1.AckNum == ack->SeqNum) {
                packet1.SeqNum = ack->AckNum;
                packet1.AckNum = generateSeqNum();
                char *to = (char *) malloc(sizeof(packet1.data));
                strncpy(to, msg + shift, sizeof(to));
                strncpy(packet1.data, to, sizeof(packet1.data));
            }
        }
    }
    
    packet1.PacketType = 0;
    memset((char *)&packet1.data, 0, sizeof(packet1.data));
    
    if (sendto (sd, (struct Packet *)&packet1, sizeof(packet1), 0, (struct sockaddr *)&server, server_len) == -1)
    {
        DieWithError ("sendto failure");
    }
    
    printTransmitted (client, server, packet1);

// 	if (strncmp(sbuf, rbuf, data_size) != 0) 
// 		printf("Data is corrupted\n");

	close(sd);
	return(0);
}

