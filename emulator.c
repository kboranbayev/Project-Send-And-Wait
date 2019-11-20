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
	
    struct Packet *temp = malloc(sizeof(struct Packet));
    sender_len = sizeof(sender);

    printf("Noise set to %d\t r = %d\n", noise, r);	

    int q = noise + 1;
    int skip = 0;
    int max_window = 0;
    
    // 2 way handshake session
    while (1) {
        while (1) {
            if (skip || (n = recvfrom (sd1, temp, sizeof(*temp), 0, (struct sockaddr *)&sender, (unsigned int *)&sender_len)) < 0) {
                DieWithError ("sd1 recvfrom error");
            } else {
                printReceived (sender, emulator, temp);
                if (temp->PacketType == 1 || temp->PacketType == 4 || temp->PacketType == 8 && (temp->WindowSize != 0)) { // SYN OR DATA OR EOT
                    struct Packet tmp;        
                    tmp = *temp;
                    receiver_len = sizeof(receiver);
                    if (noise < r) {
                        struct timeval to;
                        to.tv_sec = 1;
                        to.tv_usec = 0;
                        setsockopt(sd2, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
                        sendPacket (sd2, tmp, receiver);
                        printTransmitted (emulator, receiver, tmp);
                        if (temp->PacketType == 1 || temp->WindowSize == 0) {
                            break;
                        }
                    } else {
                        r = rand100();
                        printf("packet lost from server\n");
                    }
                } else {
                    break;
                }
            }
        }
        r = rand100();
        receiver_len = sizeof(receiver);
        
        while (1) {
            if ((n = recvfrom (sd2, temp, sizeof(*temp), 0, (struct sockaddr *)&receiver, (unsigned int *)&receiver_len)) < 0) {
                DieWithError ("sd2 recvfrom error");
            } else {
                printReceived (receiver, emulator, temp);
                if (temp->PacketType == 2 || temp->PacketType == 3) { // SYNACK OR ACK
                    struct Packet tmp;        
                    tmp = *temp;
                    sender_len = sizeof(sender);
                    if (noise < r) {
                        struct timeval to;
                        to.tv_sec = 1;
                        to.tv_usec = 0;
                        setsockopt(sd1, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
                        sendPacket (sd1, tmp, sender);
                        printTransmitted (emulator, sender, tmp);
                        if (tmp.PacketType == 2) {
                            max_window = tmp.WindowSize;
                            break;
                        } else if (tmp.WindowSize == max_window - 1) {
                            printf("Hit\n");
                            break;
                        }
                    } else {
                        r = rand100();
                        printf("packet lost from server\n");
                    }
                } else {
                    break;
                }
            }
        }
    }

    // while (1) {
    //     if ((n = recvfrom (sd1, temp, sizeof(*temp), 0, (struct sockaddr *)&sender, (unsigned int *)&sender_len)) < 0) {
    //         DieWithError ("recvfrom error");
    //     } else {
    //         printReceived (sender, emulator, temp);
    //         if (temp->PacketType == 1 || temp->PacketType == 4 || temp->PacketType == 8) { // SYN OR DATA OR EOT
    //             struct Packet tmp;
    //             tmp = *temp;
    //             receiver_len = sizeof(receiver); 
    //             if (noise < r) {
    //                 struct timeval to;
    //                 to.tv_sec = 3;
    //                 to.tv_usec = 0;
    //                 setsockopt(sd1, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
    //                 sendPacket (sd1, tmp, receiver);
    //                 printTransmitted (emulator, receiver, tmp);
    //             } else {
    //                 r = rand100();
    //                 printf("packet lost from server\n");
    //             }
    //         }
    //         if (temp->WindowSize == 0) {
    //             break;
    //         }
    //     }
    // }


    // free(temp);
    // // data client->server
    //  while (1) {
    //     if ((n = recvfrom (sd1, temp, sizeof(*temp), 0, (struct sockaddr *)&sender, (unsigned int *)&sender_len)) < 0) {
    //         DieWithError ("recvfrom error");
    //     } else {
    //         printReceived (sender, emulator, temp);
    //         if (temp->PacketType == 4 || temp->PacketType == 8) { // DATA & EOT
    //             struct Packet tmp;
    //             tmp = *temp;
    //             receiver_len = sizeof(receiver); 
    //             if (noise < noise + 1) {
    //                 struct timeval to;
    //                 to.tv_sec = 3;
    //                 to.tv_usec = 0;
    //                 setsockopt(sd2, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
    //                 sendPacket (sd2, tmp, receiver);
    //                 printTransmitted (emulator, receiver, tmp);
    //             } else {
    //                 r = rand100();
    //                 printf("packet lost from server\n");
    //             }
    //         }
    //     }
    // }

	close(sd1);
    close(sd2);
	return(0);
}
