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

uint32_t getSeqNum(const packet_t * pkt)
{
}
void setSeqNum(packet_t * pkt, uint32_t)
{
}
uint32_t getACKNum(const packet_t * pkt)
{
}
void setACKNum(packet_t * pkt, uint32_t)
{
}
uint8_t getACK(const packet_t * pkt)
{
}
void setACK(packet_t * pkt, uint8_t)
{
}
uint8_t getLast(const packet_t * pkt)
{
}
void setLast(packet_t * pkt, uint8_t)
{
}
char * getData(const packet_t * pkt)
{
}
void setData(packet_t * pkt, char * buff, size_t count)
{
}

packet_t * generatePacket(uint32_t seq_num, 
					   uint32_t ack_num, 
					   uint8_t ack, 
					   uint8_t last,
					   const char * buff,
					   size_t count);
{
	packet_t * pkt = malloc(sizeof(packet_t));

}

char** strToPackets(char * file_s)
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
char** writeTCP(int * file_p, const char * buf, size_t nbytes)
{

}