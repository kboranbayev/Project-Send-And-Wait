#include "handlers.h"

#define CLIENT_UDP_PORT		7000	// Default port
#define SERVER_UDP_PORT		8000	// Default port

int main (int argc, char **argv)
{
	int	sd1, sd2, receiver_len, sender_len, port_server = SERVER_UDP_PORT, port_client = CLIENT_UDP_PORT, n, noise = 0, r = rand100();
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
            noise = atoi(*argv);
		}
		else
		{
			fprintf (stderr,"Usage: %s [-n noise] host [port]\n", pname);
			exit(1);
		}
	}
	if (argc > 0) 
	{
        argv++;
		host = *argv;
        printf("Host %s ", host);
			if (--argc > 0)
				port_server = atoi(*++argv);
	}
	else
	{
		fprintf(stderr, "Usage:	%s [-n noise] host [port]\n", pname);
		exit(1);
	}
    
    printf("Noise set to %d\t r = %d\n", noise, r);
    
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
	memset((char *)&emulator, 0, sizeof(emulator));
	emulator.sin_family = AF_INET; 
	emulator.sin_port = htons(port_client); 
	emulator.sin_addr.s_addr = htonl(INADDR_ANY);
    
    memset((char *)&receiver, 0, sizeof(receiver));
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
        
        while (1) {
            if (noise < r) {
                if ((n = recvfrom (sd1, temp, sizeof(*temp), 0, (struct sockaddr *)&sender, &sender_len)) < 0)
                {
                    DieWithError ("recvfrom error");
                }
                printReceived (sender, emulator, temp);
                if (temp->PacketType != 99) {
                    break;
                }
            } else {
                printf("Oops packet lost\n");
            }
            r = rand100();
        }
        
        struct Packet tmp, tmp1;
        
        if (temp->PacketType == 1) {
            memset((char *)&tmp, 0, sizeof(tmp));
            tmp = *temp;
            receiver_len = sizeof(receiver);
            
            r = rand100();
            while (1) {
                struct Packet *ack = malloc(sizeof(struct Packet));
                printf("SYN Chance num %d\n", r);
                printf("ackType = %d\n", ack->PacketType);
                if (noise < r) {
                    struct timeval timeout;
                    timeout.tv_sec = 1;
                    timeout.tv_usec = 0;
                    setsockopt(sd2, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
                    while ((n = recvfrom (sd2, ack, sizeof(*ack), 0, (struct sockaddr *)&receiver, &receiver_len)) < 0)
                    {
                        n = sendto (sd2, (struct Packet *)&tmp, sizeof(tmp), 0, (struct sockaddr *)&receiver, receiver_len);
                        printTransmitted (emulator, receiver, tmp);
                    }
                    // check for timeout here?
                    printReceived (receiver, emulator, ack);
                } 
                if (ack->PacketType >= 0 && ack->PacketType <= 99) {
                    tmp1 = *ack;
                    if ((noise < r) && (sendto (sd1, (struct Packet *)&tmp1, sizeof(tmp1), 0, (struct sockaddr *)&sender, sender_len) == -1))
                    {
                        DieWithError ("sendto failure");



                    }
                    printTransmitted (emulator, sender, tmp1);
                    free(ack);
                    break;
                    
                } else {
                    printf("Oops packet lost Syn\n");
                    r = rand100();
                }
            }
            //////////// 3-way ends
        } else if (temp->PacketType == 3) { // ACKs from client to server
            memset((char *)&tmp, 0, sizeof(tmp));
            tmp = *temp;
            receiver_len = sizeof(receiver);
            while (1) {
                struct Packet *ack = malloc(sizeof(struct Packet));
                printf("Chance num %d\n", r);
                if (noise < r) {
                    if (( n = recvfrom (sd2, ack, sizeof(*ack), 0, (struct sockaddr *)&receiver, &receiver_len)) < 0)
                    {
                        n = sendto (sd2, (struct Packet *)&tmp, sizeof(tmp), 0, (struct sockaddr *)&receiver, receiver_len);
                        printTransmitted (emulator, receiver, tmp);
                    }
                    // handle ack in server
                    break;
                } else {
                    printf("Oops packet lost Syn\n");
                    r = rand100();
                }
            }
        } else if (temp->PacketType == 4) { // DATA
            memset((char *)&tmp, 0, sizeof(tmp));
            tmp = *temp;
            receiver_len = sizeof(receiver);
            while (1) {
                struct Packet *ack = malloc(sizeof(struct Packet));
                printf("Chance num %d\n", r);
                if (noise < r) {
                    if (( n = recvfrom (sd2, ack, sizeof(*ack), 0, (struct sockaddr *)&receiver, &receiver_len)) < 0)
                    {
                        n = sendto (sd2, (struct Packet *)&tmp, sizeof(tmp), 0, (struct sockaddr *)&receiver, receiver_len);
                        printTransmitted (emulator, receiver, tmp);
                    }
                    if (ack->PacketType == 0) {
                        printf("EOT not handled");
                    } else if (ack->PacketType > 0 && ack->PacketType <= 99) {
                        struct Packet tmp1 = *ack;
                        if (sendto (sd1, (struct Packet *)&tmp1, sizeof(tmp1), 0, (struct sockaddr *)&sender, sender_len) == -1)
                        {
                            DieWithError ("sendto failure");
                        }
                        printTransmitted (emulator, sender, tmp1);
                        free(ack);
                        break;
                    }
                } else {
                    printf("Oops packet lost Syn\n");
                    r = rand100();
                }
            }
        } else if (temp->PacketType == 0) { // EOT
            memset((char *)&tmp, 0, sizeof(tmp));
            tmp = *temp;
            receiver_len = sizeof(receiver);
            while (1) {
                struct Packet *ack = malloc(sizeof(struct Packet));
                printf("Chance num %d\n", r);
                if (noise < r) {
                    if (( n = recvfrom (sd2, ack, sizeof(*ack), 0, (struct sockaddr *)&receiver, &receiver_len)) < 0)
                    {
                        n = sendto (sd2, (struct Packet *)&tmp, sizeof(tmp), 0, (struct sockaddr *)&receiver, receiver_len);
                        printTransmitted (emulator, receiver, tmp);
                    }
                    break;
                } else {
                    printf("Oops packet lost Syn\n");
                    free(&tmp);
                    free(ack);
                    r = rand100();
                }
            }
        } else {
            memset((char *)&tmp, 0, sizeof(tmp));
            tmp = *temp;
            sleep(0.5);
            receiver_len = sizeof(receiver);
            
            r = rand100();
            
            while (noise > r) {
                printf("Chance num %d\n", r);
                if (sendto (sd2, (struct Packet *)&tmp, sizeof(tmp), 0, (struct sockaddr *)&receiver, receiver_len) == -1)
                {
                    DieWithError ("sendto failure");
                }
                printTransmitted (emulator, receiver, tmp);
                r = rand100();
            }
            
            struct Packet *ack = malloc(sizeof(struct Packet));
            
            r = rand100();
            while (noise > r) {
                printf("Chance num %d\n", r);
                if ((n = recvfrom (sd2, ack, sizeof(*ack), 0, (struct sockaddr *)&receiver, &receiver_len)) < 0)
                {
                    DieWithError ("recvfrom error");
                }
                printReceived (receiver, emulator, ack);
                r = rand100();
            }
            
            struct Packet tmp1 = *ack;
            
            r = rand100();
            while (noise > r) {
                printf("Chance num %d\n", r);
                if (sendto (sd1, (struct Packet *)&tmp1, sizeof(tmp1), 0, (struct sockaddr *)&sender, sender_len) == -1)
                {
                    DieWithError ("sendto failure");
                }
                printTransmitted (emulator, sender, tmp1);
                r = rand100();
            }
        }
	}
	close(sd1);
    close(sd2);
	return(0);
}
