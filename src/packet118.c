#include "packet118.h"


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
	// printf("checksum4: sum=%x, checksum=%x\n", sum, checksum);
	return checksum;
}

uint32_t getSeqNum(const byte_t * pkt)
{
	return ((uint32_t *)pkt)[SEQ_NUM_OFFSET>>5];
}

void setSeqNum(byte_t * pkt, uint32_t seqNum)
{
	// printf("setSeqNum: input seqNum=%d\n",seqNum);
	((uint32_t *)pkt)[SEQ_NUM_OFFSET>>5] = seqNum;
	// printf("setSeqNum: output seqNum=%d\n",getSeqNum(pkt));
	//SEGFAULT when trying to use pkt in here
}

uint32_t getACKNum(const byte_t * pkt)
{
	return ((uint32_t *)pkt)[ACK_NUM_OFFSET>>5];
}

void setACKNum(byte_t * pkt, uint32_t ACKNum)
{
	int index = ACK_NUM_OFFSET>>5;
	((uint32_t *)pkt)[index] = ACKNum;
	// printf("setACKNum: index=%d\n",index);
}

bool getACK(const byte_t * pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[ACK_OFFSET>>5];
	// printf("getACK: temp=%x\n",temp);
	temp = temp >> 31;
	temp = temp & 0x1;
	return temp;
}

void setACK(byte_t * pkt, bool ACK)
{
	// int offset = ACK_OFFSET;
	uint32_t temp = ACK;

	temp = temp << 31;
	// printf("setACK: temp=%08x",temp);
	((uint32_t *)pkt)[ACK_OFFSET>>5] = (((uint32_t *)pkt)[ACK_OFFSET>>5]&0x7FFFFFFF) | temp;

	// printf("setACK: ack=%x\n",((uint32_t *)pkt)[ACK_OFFSET>>5] );
}	


bool getLast(const byte_t * pkt)
{

	uint32_t temp = 0;
	if (pkt == NULL)
	{ 
		printf("getLast: pkt is NULL\n");
		return false;
	}
	temp = ((uint32_t *)pkt)[LAST_OFFSET>>5];
	// printf("getLast: temp=%x\n",temp);
	temp = temp >> 30;
	temp = temp & 0x1;
	return temp;
}

void setLast(byte_t * pkt, bool last)
{
		// int offset = ACK_OFFSET;
	uint32_t temp = last;

	temp = temp << 30;

	((uint32_t *)pkt)[LAST_OFFSET>>5] = (((uint32_t *)pkt)[LAST_OFFSET>>5]&0xBFFFFFFF) | temp;

	// printf("setLast: last=%x\n",((uint32_t *)pkt)[LAST_OFFSET>>5] );
}

bool getClose(const byte_t * pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[SHAKE_OFFSET>>5];
	// printf("getLast: temp=%x\n",temp);
	temp = temp >> 29;
	temp = temp & 0x1;
	// printf("getClose: =%d\n",temp);
	return (temp != 0);
}

void setClose(byte_t * pkt, bool last)
{
		// int offset = ACK_OFFSET;
	uint32_t temp = last;

	temp = temp << 29;

	((uint32_t *)pkt)[SHAKE_OFFSET>>5] = (((uint32_t *)pkt)[SHAKE_OFFSET>>5]&0xDFFFFFFF) | temp;

	printf("setClose: close=%d\n",getClose(pkt));
}

uint16_t getSize(const byte_t * pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[SIZE_OFFSET>>5];
	// printf("getSize: temp=%x\n",temp);
	temp = temp >> 16;
	temp = temp & 0x3FF;
	return (uint16_t)temp;
}

int setSize(byte_t * pkt, uint16_t size)
{
		// int offset = ACK_OFFSET;
	// printf("setSize: start size=%d\n",size);
	uint32_t temp = size;
	int ret = size;
	if(size > MAX_BODY_SIZE){
		temp = MAX_BODY_SIZE;
		ret = MAX_BODY_SIZE;
	}

	temp = temp << 16;
	// printf("setSize: temp=%x\n",temp);
	((uint32_t *)pkt)[SIZE_OFFSET>>5] = (((uint32_t *)pkt)[SIZE_OFFSET>>5]&0xFC00FFFF) | temp;
	// printf("setSize: size=%d\n",getSize(pkt));
	// printf("setSize: size=%x\n",((uint32_t *)pkt)[SIZE_OFFSET>>5] );
	return ret;
}

uint16_t getChecksum(const byte_t *pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[CHECKSUM_OFFSET>>5];
	// printf("getChecksum: temp=%x\n",temp);
	temp = temp & 0xFFFF;
	return (uint16_t)temp;
}

void setChecksum(byte_t * pkt, uint16_t checksum)
{
	uint32_t temp = checksum & 0x0000FFFF;
	((uint32_t *)pkt)[CHECKSUM_OFFSET>>5] = (((uint32_t *)pkt)[CHECKSUM_OFFSET>>5]&0xFFFF0000) | temp;

}

byte_t * getBody(const byte_t * pkt)
{
	//strdup?
	return strdup(&(pkt[BODY_OFFSET>>3]));
}

int setBody(byte_t * pkt, byte_t * buff, size_t count)
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

byte_t * generatePacket(byte_t * pkt,
					   uint32_t seq_num, 
					   uint32_t ack_num, 
					   bool ack, 
					   bool last,
					   bool close,
					   byte_t * buff,
					   size_t count)
{
	//pkt should be allocated before entering this function;
	if(pkt == NULL)
	{
		return NULL;
	}
	else
	{
	uint16_t cs = 0;

	memset(pkt, 0, PACKET_SIZE);

	setSeqNum(pkt, seq_num);
	setACKNum(pkt, ack_num);
	setACK(pkt, ack);
	setLast(pkt, last);
	setClose(pkt, close);
	setSize(pkt, count);
	setBody(pkt, buff, count);
	cs = checksum(pkt, PACKET_SIZE);
	setChecksum(pkt, cs);

	//finish
	return pkt;
	}
}

void printPacket(byte_t * pkt)
{
	if(pkt == NULL){
		//bad things
		printf("printPacket: was not given a valid string\n");
	}
	else{
		int i = 0;
		byte_t * body = getBody(pkt);
		uint16_t cs_msg = getChecksum(pkt);
		uint16_t cs_valid = checksum(pkt,PACKET_SIZE);

		printf("printPacket:\n");
		printf("\tpkt bits\n");
     	for(i = 0; i < PACKET_SIZE/32; i+=8)
     	{
            printf("%08x %08x %08x %08x ",((uint32_t *)pkt)[i], ((uint32_t *)pkt)[i+1], ((uint32_t *)pkt)[i+2], ((uint32_t *)pkt)[i+3]);
        	printf("%08x %08x %08x %08x\n",((uint32_t *)pkt)[i+4], ((uint32_t *)pkt)[i+5], ((uint32_t *)pkt)[i+6], ((uint32_t *)pkt)[i+7]);
     	}
		printf("\tseq_num=%x\n", getSeqNum(pkt));
		printf("\tack_num=%x\n", getACKNum(pkt));
		printf("\tack=%d\n", getACK(pkt));
		printf("\tlast=%d\n",getLast(pkt));
		printf("\tclose=%d\n",getClose(pkt));
		printf("\tsize=%d\n",getSize(pkt));
		printf("\tchecksum:\n");
		printf("\t\tcs_msg=%x\n",cs_msg);
		printf("\t\tcs_valid==0?=%x\n", cs_valid);
		printf("\tbody=\n\t:");
		for(i = 0; i < getSize(pkt); i++)
		{
			printf("%c",body[i]);
		}
		printf("\n");
		printf("printPacket: END\n");
		free(body);
	}
}

void freePacket(byte_t * pkt)
{
	free(pkt);
	pkt = NULL;
}

void freePackets(byte_t **pkts)
{
	int i = 0;
	for(i = 0; pkts[i] != 0; i++)
	{
		freePacket(pkts[i]);
	}
	free(pkts);
}

//need to support binary files by using memcpy instead of strncpy

byte_t** bufToPackets(byte_t * buf, uint32_t nbytes, uint32_t c_wnd)
{
	int numPackets = 0;
	int i = 0;
	byte_t **packetArray = NULL;

	if(buf == NULL){
		return NULL; //maybe I should return an allocated byte_t**
	}


	numPackets = (nbytes+(MAX_BODY_SIZE-1))/MAX_BODY_SIZE; //rounds up
	// printf("bufToPackets, number of packets=%d\n",numPackets);
	//allocate space
	// printf("bufToPackets, before packetarray malloc\n");
	packetArray = (byte_t**)malloc(sizeof(byte_t*) * (numPackets+1));
	memset(packetArray, 0, sizeof(byte_t*)*(numPackets+1));
	for(i = 0; i < numPackets; i++){
		// printf("bufToPackets, allocating string %d\n", i);
		packetArray[i] = (byte_t*)malloc(sizeof(byte_t) * (PACKET_SIZE));
		memset(packetArray[i], '\0', sizeof(byte_t)*(PACKET_SIZE));
	}

	//divide up the file string
	for(i = 0; i < numPackets; i++){
		int pieceLen = MAX_BODY_SIZE;
		int seqNum = (i*PACKET_SIZE)%(2*c_wnd);
		printf("bufToPackets: seqNum=%d\n", seqNum);
		//really this should not know the sequence number
		if(nbytes < MAX_BODY_SIZE)
		{
			pieceLen = nbytes;
			nbytes = 0;
			generatePacket(packetArray[i], seqNum,seqNum,0,1,0,&(buf[i*MAX_BODY_SIZE]), pieceLen);
			// printf("bufToPackets, copy to pkt#=%d, nbytes=%d, pieceLen=%d\n",i, nbytes, pieceLen);
		}
		else{
			nbytes -= MAX_BODY_SIZE;
			generatePacket(packetArray[i], seqNum,seqNum,0,0,0,&(buf[i*MAX_BODY_SIZE]), pieceLen);
			// printf("bufToPackets, copy to pkt#=%d, nbytes=%d, pieceLen=%d\n",i, nbytes, pieceLen);
		}	
		// printPacket(packetArray[i]);
	}

	return packetArray;
}
