#include "tcp118.h"

uint16_t checksum(const uint8_t * addr, uint32_t count)
{
	int i = 0;
	uint32_t sum = 0, checksum = 0;

	while(i < (count - 1)) {
		sum += ((uint16_t *) addr)[i/2];
		sum = (sum & 0xFFFF) + (sum >> 16); //just to avoid any possible overflow
		i+=2;
		// if(i < 4) printf("checksum1:i=%d, sum=%x, checksum=%x\n",i, sum, checksum);
	}

	if(i < count)
	{
		sum += *((uint8_t *) addr);
		// printf("checksum2: sum=%x, checksum=%x\n", sum, checksum);
	}

	while (sum >> 16)
	{
		sum = (sum & 0xFFFF) + (sum >> 16);
		// printf("checksum3: sum=%x, checksum=%x\n", sum, checksum);
	}

	checksum = ~sum;
	printf("checksum4: sum=%x, checksum=%x\n", sum, checksum);
	return checksum;
}

uint32_t getSeqNum(const char * pkt)
{
	return ((uint32_t *)pkt)[SEQ_NUM_OFFSET>>5];
}

void setSeqNum(char * pkt, uint32_t seqNum)
{
	((uint32_t *)pkt)[SEQ_NUM_OFFSET>>5] = seqNum;
	//SEGFAULT when trying to use pkt in here
}

uint32_t getACKNum(const char * pkt)
{
	return ((uint32_t *)pkt)[ACK_NUM_OFFSET>>5];
}

void setACKNum(char * pkt, uint32_t ACKNum)
{
	int index = ACK_NUM_OFFSET>>5;
	((uint32_t *)pkt)[index] = ACKNum;
	printf("setACKNum: index=%d\n",index);
}

bool getACK(const char * pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[ACK_OFFSET>>5];
	printf("getACK: temp=%x\n",temp);
	temp = temp >> 31;
	temp = temp & 0x1;
	return temp;
}

void setACK(char * pkt, bool ACK)
{
	// int offset = ACK_OFFSET;
	uint32_t temp = ACK;

	temp = temp << 31;
	// printf("setACK: temp=%08x",temp);
	((uint32_t *)pkt)[ACK_OFFSET>>5] = (((uint32_t *)pkt)[ACK_OFFSET>>5]&0x7FFFFFFF) | temp;

	printf("setACK: ack=%x\n",((uint32_t *)pkt)[ACK_OFFSET>>5] );
}	


bool getLast(const char * pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[LAST_OFFSET>>5];
	printf("getLast: temp=%x\n",temp);
	temp = temp >> 30;
	temp = temp & 0x1;
	return temp;
}

void setLast(char * pkt, bool last)
{
		// int offset = ACK_OFFSET;
	uint32_t temp = last;

	temp = temp << 30;

	((uint32_t *)pkt)[LAST_OFFSET>>5] = (((uint32_t *)pkt)[LAST_OFFSET>>5]&0xBFFFFFFF) | temp;

	printf("setLast: last=%x\n",((uint32_t *)pkt)[LAST_OFFSET>>5] );
}

uint16_t getSize(const char * pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[SIZE_OFFSET>>5];
	printf("getSize: temp=%x\n",temp);
	temp = temp >> 16;
	temp = temp & 0x2FF;
	return (uint16_t)temp;
}

int setSize(char * pkt, uint16_t size)
{
		// int offset = ACK_OFFSET;
	uint32_t temp = size;
	int ret = size;
	if(size > MAX_BODY_SIZE){
		temp = MAX_BODY_SIZE;
		ret = MAX_BODY_SIZE;
	}

	temp = temp << 16;

	((uint32_t *)pkt)[SIZE_OFFSET>>5] = (((uint32_t *)pkt)[SIZE_OFFSET>>5]&0xFC00FFFF) | temp;

	printf("setSize: size=%x\n",((uint32_t *)pkt)[SIZE_OFFSET>>5] );
}

uint16_t getChecksum(const char *pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[CHECKSUM_OFFSET>>5];
	printf("getChecksum: temp=%x\n",temp);
	temp = temp & 0xFFFF;
	return (uint16_t)temp;
}

void setChecksum(char * pkt, uint16_t checksum)
{
	uint32_t temp = checksum & 0x0000FFFF;
	((uint32_t *)pkt)[CHECKSUM_OFFSET>>5] = (((uint32_t *)pkt)[CHECKSUM_OFFSET>>5]&0xFFFF0000) | temp;

}

char * getBody(const char * pkt)
{
	//strdup?
	return strdup(&(pkt[BODY_OFFSET>>3]));
}

int setBody(char * pkt, char * buff, size_t count)
{

	int i = 0; 
	int copied = count;
	if(pkt == NULL || buff == NULL) return -1;
	if(getSize(pkt) != copied){
		printf("setData: remember packet size parameter and data byte count must ==\n");
	}
	if(copied > MAX_BODY_SIZE) {
		copied = MAX_BODY_SIZE;
	}
	//maybe check to make sure pkt and buff != null;
	memcpy(&(pkt[BODY_OFFSET>>3]), buff, copied);
	return copied;
}

char * generatePacket(uint32_t seq_num, 
					   uint32_t ack_num, 
					   bool ack, 
					   bool last,
					   char * buff,
					   size_t count)
{
	char * pkt = malloc(PACKET_SIZE);
	uint16_t cs = 0;
	memset(pkt, 0, PACKET_SIZE);

	setSeqNum(pkt, seq_num);
	setACKNum(pkt, ack_num);
	setACK(pkt, ack);
	setLast(pkt, last);
	setSize(pkt, count);
	setBody(pkt, buff, count);
	cs = checksum(pkt, PACKET_SIZE);
	setChecksum(pkt, cs);

	//finish
	return pkt;
}

void printPacket(char * pkt)
{
	if(pkt == NULL){
		//bad things
		printf("printPacket: was not given a valid string\n");
	}
	else{
		char * body = getBody(pkt);
		uint16_t cs_msg = getChecksum(pkt);
		uint16_t cs_valid = checksum(pkt,PACKET_SIZE);

		printf("printPacket:\n");
		printf("\tseq_num=%x\n", getSeqNum(pkt));
		printf("\tack_num=%x\n", getACKNum(pkt));
		printf("\tack=%d\n", getACK(pkt));
		printf("\tlast=%d\n",getLast(pkt));
		printf("\tsize=%d\n",getSize(pkt));
		printf("\tchecksum:\n");
		printf("\t\tcs_msg=%x\n",cs_msg);
		printf("\t\tcs_valid==0?=%x\n", cs_valid);
		printf("\tbody=\n\t:%s\n",body);
		printf("printPacket: END\n");
		free(body);
	}
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