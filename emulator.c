/**
 * 
 * @author Kuanysh Boranbayev
 * @author Parm Dhaliwal
 * @date November 27, 2019
 */
#include "handlers.h"

#define CLIENT_UDP_PORT		    7000	// Default port
#define SERVER_UDP_PORT		    8000	// Default port
#define MILLI_TO_SEC            1000    // 1000 milliseconds = 1 second

int average_delay = 0;

void *delayThread() {
    sleep( average_delay / MILLI_TO_SEC );
} 


int main (int argc, char **argv)
{
	int	sd1, sd2, receiver_len, sender_len, n, noise = 0, r = rand100();
	struct	sockaddr_in receiver, sender, emulator;
    struct hostent *hp;
    char *pname, *conf_file;
    struct IP_PORT_ID *config = malloc(2*sizeof(struct IP_PORT_ID *));

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

        printf("server: %s:%d\n", config[0].ip_port.ip_address, config[0].ip_port.port);
        printf("network: %s:%d\n", config[1].ip_port.ip_address, config[1].ip_port.port);
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
	memset((char *)&emulator, 0, sizeof(emulator));
	emulator.sin_family = AF_INET; 
	emulator.sin_port = htons(7000); 
	emulator.sin_addr.s_addr = htonl(INADDR_ANY);
    
    memset((char *)&receiver, 0, sizeof(receiver));
	receiver.sin_family = AF_INET; 
	receiver.sin_port = htons(config[0].ip_port.port);
    
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
    pthread_t tid;
    
    while (1) {
        while (1) {
            puts("LISTEN => CLIENT");
            if ((n = recvfrom (sd1, temp, sizeof(*temp), 0, (struct sockaddr *)&sender, (unsigned int *)&sender_len)) < 0) {
                DieWithError ("RECVFROM => CLIENT OFFLINE");
            } else {
                printNetworkReceived (sender, emulator, temp);
                if ((temp->PacketType == 1 || temp->PacketType == 4) && (temp->WindowSize != 0)) { // SYN OR DATA OR EOT
                    struct Packet tmp;        
                    tmp = *temp;
                    if ((noise < 101 && noise > 89)) {
                        r = rand100();
                        puts("PACKET LOST => Client");
                    } else if (temp->PacketType == 4 && (temp->re == 1) && (noise <= 100 || noise >= 90)) {
                        sendPacket (sd2, tmp, receiver);
                        printNetworkReTransmitted (emulator, receiver, tmp);
                        if (temp->last == 1) {
                            break;
                        }
                    } else if ((temp->re == 1) && temp->PacketType == 1 && noise != 100) {
                        sendPacket (sd2, tmp, receiver);
                        printNetworkReTransmitted (emulator, receiver, tmp);
                        break;
                    } else if (noise < r) {
                        while (average_delay != 0)
                        {
                            printf("tid = %ld\n", tid);
                            if (pthread_create(&tid, NULL, delayThread, (void *)&tid) != 0) {
                               printf("Failed to create thread\n");
                               tid++;
                            } else {
                                sendPacket (sd2, tmp, receiver);
                                printNetworkTransmitted (emulator, receiver, tmp);
                                pthread_join(tid, NULL);
                                break;
                            }
                        }
                        if (average_delay == 0) {
                            sendPacket (sd2, tmp, receiver);
                            printNetworkTransmitted (emulator, receiver, tmp);
                        }
                        if (temp->PacketType == 1 || temp->WindowSize == 0 || temp->last == 1) {
                            break;
                        }                       
                    } else {
                        r = rand100();
                        puts("PACKET LOST => Client");
                    }
                } else if (temp->PacketType == 8) {
                    struct Packet tmp;        
                    tmp = *temp;
                    sendPacket (sd2, tmp, receiver);
                    printNetworkReTransmitted (emulator, receiver, tmp);
                    break;
                } else {
                    break;
                }
            }
        }
        r = rand100();

        r = rand100();

        r = rand100();
        receiver_len = sizeof(receiver);
        
        while (1) {
            struct timeval to;
            to.tv_sec = 5;
            setsockopt(sd2, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
            puts("LISTEN => SERVER");
            if ((n = recvfrom (sd2, temp, sizeof(*temp), 0, (struct sockaddr *)&receiver, (unsigned int *)&receiver_len)) < 0) {
                DieWithError ("RECVFROM => SERVER OFFLINE");
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
                        while (average_delay != 0)
                        {
                            printf("tid = %ld\n", tid);
                            if (pthread_create(&tid, NULL, delayThread, (void *)&tid) != 0) {
                               printf("Failed to create thread\n");
                               tid++;
                            } else {
                                sendPacket (sd1, tmp, sender);
                                printNetworkTransmitted (emulator, sender, tmp);
                                pthread_join(tid, NULL);
                                break;
                            }
                        }
                        if (average_delay == 0) {
                            sendPacket (sd1, tmp, sender);
                            printNetworkTransmitted (emulator, sender, tmp);
                        }
                        
                        if (tmp.PacketType == 2) {
                            break;
                        } else if (tmp.last == 1) {
                            break;
                        }
                    } else {
                        r = rand100();
                        puts("PACKET LOST => Server");
                    }
                } else if (temp->PacketType == 5) { // NAK
                    sendPacket (sd1, *temp, sender);
                    printNetworkTransmitted (emulator, sender, *temp);
                    break;
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