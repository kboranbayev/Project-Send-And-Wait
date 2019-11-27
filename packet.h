/**
 * 
 * @author Kuanysh Boranbayev
 * @date November 27, 2019
 */

struct Packet
{
	int PacketType;
	unsigned int SeqNum;
	unsigned int AckNum;
	int WindowSize;
    char data[5];
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
    char ip_address[20];
    int port;
};

struct IP_PORT_ID
{
    struct IP_PORT ip_port;
};
