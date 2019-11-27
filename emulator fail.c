#include "handlers.h"

#define CLIENT_UDP_PORT		7000	// Default port
#define SERVER_UDP_PORT		8000	// Default port

int main (int argc, char **argv)
{
	int	sd1, sd2, receiver_len, sender_len, n, noise = 0, average_delay = 0, r = rand100();
	struct	sockaddr_in receiver, sender, emulator;
    struct hostent *hp;
    char *pname, *conf_file;
    struct IP_PORT_ID *config = malloc(2*sizeof(struct IP_PORT_ID *));
    struct timeval start, end;

    pname = argv[0];
    argc--;
    argv++;

    if (argc > 0) {
        for (int i = 0; i < argc; i++) {
            if (!strcmp(argv[i], "-n")) {
                noise = atoi(argv[++i]);
            } else if (!strcmp(argv[i], "-d")) {
                average_delay = atoi(argv[++i]);
            } else if (!strcmp(argv[i], "-f")) {
                conf_file = argv[i + 1];
            }
        }
        
        config = getIPsAndPorts(conf_file);
        
        printf("noise = %d%% delay = %d ms\n", noise, average_delay);

    } else {
        fprintf(stderr, "Usage:	%s [-n noise] [-d delay] [-f config.txt]\n", pname);
        exit(1);
    }

    
	// Create a datagram socket for client
	if ((sd1 = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		DieWithError ("Can't create a socket");
	}
	
	// Create a datagram socket for server
	if ((sd2 = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		DieWithError ("Can't create a socket");
	}


	// Bind an address to the socket
	memset((char *)&emulator, 0, sizeof(config[1].ip_port.ip_address));
	emulator.sin_family = AF_INET; 
	emulator.sin_port = htons(config[1].ip_port.port); 
	emulator.sin_addr.s_addr = htonl(INADDR_ANY);
    
    memset((char *)&receiver, 0, sizeof(receiver));
	receiver.sin_family = AF_INET; 
	receiver.sin_port = htons(config[0].ip_port.port);
    
    
    //if ((hp = gethostbyname(config[0].ip_port.ip_address)) == NULL)
    if ((hp = gethostbyname(config[0].ip_port.ip_address)) == NULL)
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
    gettimeofday(&start, NULL);
    while (1) {
        while (1) {
            printf("listen for client\n");
            if ((n = recvfrom (sd1, temp, sizeof(*temp), 0, (struct sockaddr *)&sender, (unsigned int *)&sender_len)) < 0) {
                DieWithError ("sd1 recvfrom error");
            } else {
                printNetworkReceived (sender, emulator, temp);
                if ((temp->PacketType == 1 || temp->PacketType == 4) && (temp->WindowSize != 0)) { // SYN OR DATA OR EOT
                    struct Packet tmp;        
                    tmp = *temp;
                    
                    if ((temp->PacketType == 4 && (temp->re == 1) && noise != 100) || (temp->PacketType == 4 && temp->WindowSize == 1)) {
                        sendPacket (sd2, tmp, receiver);
                        printNetworkReTransmitted (emulator, receiver, tmp);
                    } else if ((temp->re == 1) && temp->PacketType == 1 && noise != 100) {
                        sendPacket (sd2, tmp, receiver);
                        printNetworkReTransmitted (emulator, receiver, tmp);
                        break;
                    } else if (noise < r) {
                        sleep(average_delay/1000);
                        sendPacket (sd2, tmp, receiver);
                        printNetworkTransmitted (emulator, receiver, tmp);
                        if (temp->PacketType == 1 || temp->WindowSize == 0 || temp->last == 1) {
                            break;
                        }
                    } else if (noise >=90 || noise <= 100) {
                        r = rand100();
                        printf("packet lost from client\n");
                        sleep(1.5);
                        struct Packet tmp;        
                        tmp = *temp;
                        tmp.PacketType = 6;
                        sendPacket (sd2, tmp, receiver);
                    } else {
                        r = rand100();
                        printf("packet lost from client\n");
                        sleep(1.5);
                        struct Packet tmp;        
                        tmp = *temp;
                        tmp.PacketType = 6;
                        sendPacket (sd2, tmp, receiver);
                    }
                } else if (temp->PacketType == 8) {
                    struct Packet tmp;        
                    tmp = *temp;
                    tmp.PacketType = 6;
                    sendPacket (sd2, tmp, receiver);
                    printNetworkReTransmitted (emulator, receiver, tmp);
                    break;
                } else {
                    break;
                }
            }
        }
        r = rand100();

//        r = rand100();

        r = rand100();
        receiver_len = sizeof(receiver);
        gettimeofday(&end, NULL);
        printf("rtt = %d\n", delay(start, end));
        while (1) {
            struct timeval to;
            
            to.tv_sec = delay(start, end);
            setsockopt(sd2, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
            printf("listen for server\n");
            if ((n = recvfrom (sd2, temp, sizeof(*temp), 0, (struct sockaddr *)&receiver, (unsigned int *)&receiver_len)) < 0) {
                DieWithError ("sd2 recvfrom error");
                break;
            } else {
                printNetworkReceived (receiver, emulator, temp);
                if (temp->PacketType == 2 || temp->PacketType == 3) { // SYNACK OR ACK
                    struct Packet tmp;        
                    tmp = *temp;
                    sender_len = sizeof(sender);
                    if ((temp->re == 1 || temp->last == 1) && noise != 100) {
                        sendPacket (sd1, tmp, sender);
                        printNetworkReTransmitted (emulator, sender, tmp);
                        break;
                    } if (noise < r) {
                        sleep(average_delay/1000);
                        sendPacket (sd1, tmp, sender);
                        printNetworkTransmitted (emulator, sender, tmp);
                        if (tmp.PacketType == 2) {
                            break;
                        } else if (tmp.last == 1) {
                            break;
                        }
                    } else if (noise >= 90 || noise <= 100) {
                        r = rand100();
                        printf("packet lost from server\n");
                    } else {
                        r = rand100();
                        printf("packet lost from server\n");
                        sleep(1.5);
                    }
                } else {
                    break;
                }
            }
        }
    }
	close(sd1);
    close(sd2);
	return(0);
}