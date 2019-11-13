struct Packet
{
	int PacketType;
	unsigned int SeqNum;
	unsigned int AckNum;
	int WindowSize;
    char data[5];
};

struct PacketRTT
{
    struct Packet packet;
    struct timeval start;
    long int delay;
};

struct PacketByte
{
    struct Packet packet;
    int bytes;
};
