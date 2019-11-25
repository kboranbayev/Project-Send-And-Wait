#include "handlers.h"

// #define CLIENT_UDP_PORT		7000	// Default port
// #define SERVER_UDP_PORT		8000	// Default port

int main (int argc, char **argv)
{
	int	sd1, sd2, receiver_len, sender_len, n, noise = 0, delay = 0, r = rand100();
	struct	sockaddr_in receiver, sender, emulator;
    struct hostent *hp;
    char *pname, *conf_file;
    struct IP_PORT_ID *config = malloc(2*sizeof(struct IP_PORT_ID *));
    
    // Express server
    int node_fd, port_express = 3000;
    struct sockaddr_in express;
    struct hostent *node_hp;


    pname = argv[0];
    argc--;
    argv++;

    if (argc > 0) {
        for (int i = 0; i < argc; i++) {
            if (!strcmp(argv[i], "-n")) {
                noise = atoi(argv[++i]);
            } else if (!strcmp(argv[i], "-d")) {
                delay = atoi(argv[++i]);
            } else if (!strcmp(argv[i], "-f")) {
                conf_file = argv[i + 1];
            }
        }
        
        config = getIPsAndPorts(conf_file);
        
        printf("ip=%s port=%d\n", config[0].ip_port.ip_address, config[0].ip_port.port);
        printf("ip=%s port=%d\n", config[1].ip_port.ip_address, config[1].ip_port.port);
        printf("noise = %d%% delay = %d ms\n", noise, delay);

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

    // Create a tcp socket for expess server
	if ((node_fd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
	{
		DieWithError ("Can't create a socket for express server");
	}

    /* lookup the ip address */
    if((node_hp = gethostbyname("127.0.0.1")) == NULL){
		herror("gethostbyname");
		exit(1);
	}


    /* fill in the structure */
    //memset(&express,0,sizeof(express));
    //express.sin_family = AF_INET;
    //express.sin_port = htons(port_express);
    //memcpy(&express.sin_addr.s_addr,node_hp->h_addr,node_hp->h_length);

    // node_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	

    /* connect the socket */
    // if (connect(node_fd,(struct sockaddr *)&express,sizeof(express)) < 0) {
    //     error("ERROR connecting");
    // }
    //char buff[1024];
    //char postdata[1024] = "param1=value1&param2=value";  /* Change param and value here */

    // sprintf(buff,"POST %s HTTP1.1\r\nAccept: */*\r\nAccept-Language: en-us\r\nContent-Type: application/x-www-form-urlencoded\r\nAccept-Encoding: gzip,deflate\r\nContent-Length: %d\r\nConnection: keep-alive\r\n\r\n%s",request,strlen(postdata),postdata);
    // if(write(node_fd,buff,sizeof(buff)) == -1) {
    //     perror("write");
    // }

    //char *header = "POST /packet_lost HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\nContent-Length: 1024\r\nConnection: keep-alive\r\n\r\nparam1=lost&param2=forward\n";
    //send(node_fd,header,strlen(header),0);
    
    //recv(node_fd, buff, sizeof(buff), 0);
    //printf("%s\n", buff);
	// Bind an address to the socket
	memset((char *)&emulator, 0, sizeof(emulator));
	emulator.sin_family = AF_INET; 
	emulator.sin_port = htons(config[1].ip_port.port); 
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
                    
                    if (temp->PacketType == 4 && (temp->re == 1) && noise != 100) {
                        sendPacket (sd2, tmp, receiver);
                        printNetworkReTransmitted (emulator, receiver, tmp);
                    } else if ((temp->re == 1) && temp->PacketType == 1 && noise != 100) {
                        sendPacket (sd2, tmp, receiver);
                        printNetworkReTransmitted (emulator, receiver, tmp);
                        break;
                    } else if (noise < r) {
                        sleep(delay/1000);
                        sendPacket (sd2, tmp, receiver);
                        printNetworkTransmitted (emulator, receiver, tmp);
                        if (temp->PacketType == 1 || temp->WindowSize == 0 || temp->last == 1) {
                            break;
                        }
                    } else {
                        r = rand100();
                        printf("packet lost from client\n");
                        
                        // req_t req;
                        // int ret = requests_init(&req);
                        // if (ret) { return 1;}
                        // char *packet[] = { "Content-Type: kuanysh", "temp->PacketType"};
                        // int packet_size = sizeof(packet)/sizeof(char *);
                        // char *body = requests_url_encode(&req, packet, packet_size);
                        // printf("%s", body);
                        // //requests_post(&req, "http://localhost:3000/packet_lost", body);
                        // requests_get(&req, "http://localhost:3000/packet_lost");
                        // printf("Request URL: %s\n", req.url);
                        // printf("Response Code: %lu\n", req.code);
                        // printf("Response Size: %zu\n", req.size);
                        // printf("Response Body:\n%s", req.text);
                        // curl_free(body);
                        // requests_close(&req);
                    }
                } else if (temp->PacketType == 8) {
                    struct Packet tmp;        
                    tmp = *temp;
                    sendPacket (sd2, tmp, receiver);
                    printTransmitted (emulator, receiver, tmp);
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
                        sleep(delay/1000);
                        sendPacket (sd1, tmp, sender);
                        printNetworkTransmitted (emulator, sender, tmp);
                        if (tmp.PacketType == 2) {
                            break;
                        } else if (tmp.last == 1) {
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
	close(sd1);
    close(sd2);
    close(node_fd);
	return(0);
}
