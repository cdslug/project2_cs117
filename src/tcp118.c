#include "tcp118.h"

uint16_t checksum(const uint8_t * addr, uint32_t count)
{
	int i = 0;
	uint32_t sum = 0, checksum = 0;

	while(count > 1) {
		sum = sum + (uint16_t) *addr + 1;
		count -= 2;
	}

	if(count > 0)
	{
		sum += *((uint8_t *) addr);
	}

	while (sum >> 16)
	{
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	checksum = ~sum;
	return checksum;
}

uint32_t getSeqNum(const char * pkt)
{
return 0;
}
void setSeqNum(char * pkt, uint32_t SeqNum)
{
	int i = 0;
	for( i = 0; i < sizeof(SeqNum); i++){
		pkt[SEQ_NUM_OFFSET+i] = (SeqNum >> (8*(sizeof(SeqNum)-i-1)))&(0xF);
	}
}
uint32_t getACKNum(const char * pkt)
{
	return 0;
}
void setACKNum(char * pkt, uint32_t ACKNum)
{
	int i = 0;
	for( i = 0; i < sizeof(ACKNum); i++){
		pkt[ACK_NUM_OFFSET+i] = (ACKNum >> (8*(sizeof(ACKNum)-i-1)))&(0xF);
	}
}
uint16_t getACK(const char * pkt)
{
	return 0;
}
void setACK(char * pkt, uint16_t ACK)
{
	int offset = ACK_OFFSET;
	pkt[offset/8] = pkt[offset/8]&((ACK == 1) << (8*sizeof(ACK)-offset%(8*sizeof(ACK))));
}
uint16_t getLast(const char * pkt)
{
	return 0;
}
void setLast(char * pkt, uint16_t last)
{
	int offset = LAST_OFFSET;
	pkt[offset/8] = pkt[offset/8]&((last == 1) << (8*sizeof(last)-offset%(8*sizeof(last))));
}
char * getData(const char * pkt)
{
	return 0;
}
int setData(char * pkt, char * buff, size_t count)
{
	int i = 0; 
	//maybe check to make sure pkt and buff != null;
	if(pkt == NULL || buff == NULL) return -1;

	for(i = 0; i < count && i < MAX_BODY_SIZE; i++)
	{
		pkt[BODY_OFFSET+i] = buff[i];
	}
	return count > MAX_BODY_SIZE ? MAX_BODY_SIZE : count;
}

packet_t * generatePacket(uint32_t seq_num, 
					   uint32_t ack_num, 
					   uint8_t ack, 
					   uint8_t last,
					   const char * buff,
					   size_t count)
{
	packet_t * pkt = malloc(sizeof(packet_t));
	memset(pkt, 0, sizeof(packet_t));
	//finish
	return pkt;
}

char** strToPackets(const char * file_s)
{
	int fileSize = 0;
	int numPackets = 0;
	int i = 0;
	char **packetArray = NULL;

	if(file_s == NULL){
		return NULL; //maybe I should return an allocated char**
	}

	fileSize = strlen(file_s);
	numPackets = (fileSize+(MAX_BODY_SIZE-1))/MAX_BODY_SIZE; //rounds up
	printf("fTP, number of packets=%d\n",numPackets);
	//allocate space
	printf("fTP, before packetarray malloc\n");
	packetArray = (char**)malloc(sizeof(char*) * (numPackets+1));
	memset(packetArray, 0, sizeof(char*)*(numPackets+1));
	for(i = 0; i < numPackets; i++){
		printf("fTP, allocating string %d\n", i);
		packetArray[i] = (char*)malloc(sizeof(char) * (MAX_BODY_SIZE+1));
		memset(packetArray[i], '\0', sizeof(char)*(MAX_BODY_SIZE+1));
	}

	//divide up the file string
	for(i = 0; i < numPackets; i++){
		printf("fTP, copying file to packet %d\n",i);
		strncpy(packetArray[i], &(file_s[i*MAX_BODY_SIZE]), MAX_BODY_SIZE);
	}

	return packetArray;
}

// INPUT: 
// OUTPUT: 
// RETURNS: number of bytes written
// ABOUT: the main function for 
int writeTCP(int * file_p, const char * buf, size_t nbytes)
{
	return 0;
}