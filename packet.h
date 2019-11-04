struct Packet
{
	int PacketType;
	unsigned int SeqNum;
	unsigned int AckNum;
    	char data[5];
	int WindowSize;
};
