/**
 * 
 * @author Kuanysh Boranbayev
 * @author Parm Dhaliwal
 * @date November 27, 2019
 */

#define PAYLOAD_LEN     500 //  payload length set to 500 bytes per packet
#define ADDRESS_LEN     20  //  string length for IP address
#define TIMEOUT         10  //  default timeout set to 10 seconds

struct Packet
{
	int PacketType;
	unsigned int SeqNum;
	unsigned int AckNum;
	long int WindowSize;
    char data[PAYLOAD_LEN];
    int last;
    int re;
};

struct PacketRTT
{
    struct Packet packet;
    long int delay;
};

struct PacketSent
{
    struct Packet packet;
    int acked;
};

struct PacketPriority
{
    struct Packet packet;
    int priority;
};

struct PacketByte
{
    struct Packet packet;
    int bytes;
};

struct IP_PORT
{
    char ip_address[ADDRESS_LEN];
    int port;
};

struct IP_PORT_ID
{
    struct IP_PORT ip_port;
};
