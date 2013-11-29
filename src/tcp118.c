#include "tcp118.h"

cwnd_t* cwnd_init(cwnd_t *cwnd)
{
	int i = 0;

	cwnd = malloc(sizeof(cwnd_t));

	cwnd->c_wnd = C_WND;
	cwnd->size = C_WND * 2;
	cwnd->last_seq = 0;
	cwnd->next_seq = 0;
	cwnd->acks = malloc(sizeof(uint8_t)*C_WND*2);
	memset(cwnd->acks, 0, sizeof(uint8_t) * (C_WND * 2));
	cwnd->packets = (char **)malloc(sizeof(char*)*(C_WND * 2));

	for(i = 0; i < (C_WND * 2)/PACKET_SIZE; i++)
	{
		cwnd->packets[i] = malloc(sizeof(char)*PACKET_SIZE);
	}

	// printf("cwnd_init: start print\n");
	// printf("\tc_wnd=%d\n",cwnd->c_wnd);
	// printf("\tsize=%d\n",cwnd->size);
	// printf("\tlast_seq=%d\n",cwnd->last_seq);
	// printf("\tnext_seq=%d\n",cwnd->next_seq);
	// printf("cwnd_init: end print\n");

	return cwnd;
}

void cwnd_free(cwnd_t *cwnd)
{
	int i = 0;
	for(i = 0; i < (cwnd->size)/PACKET_SIZE; i++)
	{
		free(cwnd->packets[i]);
	}
	free(cwnd->packets);
	free(cwnd);
}	

uint32_t cwnd_nextMss(cwnd_t *cwnd)
{
	// printf("cwnd_nextMss: nextSeq=%d\n",cwnd->next_seq);
	//mod is just in case
	uint32_t next = (cwnd->next_seq/PACKET_SIZE) % cwnd->size;
	// printf("cwnd_nextMss: next=%d\n",next);
	return next;
}

uint32_t cwnd_lastMss(cwnd_t *cwnd)
{
	//mod is just in case
	return (cwnd->last_seq/PACKET_SIZE) % cwnd->size;
}

bool cwnd_getAck(cwnd_t *cwnd, uint32_t seqNum)
{
	if(cwnd->acks[seqNum%cwnd->size] == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}	

bool cwnd_checkIn(cwnd_t *cwnd, uint32_t seqNum)
{
	return false;
}

bool cwnd_checkAdd(cwnd_t *cwnd, uint32_t seqNum)
{
	return false;
}

bool cwnd_addPkt(cwnd_t *cwnd, char *buf)
{
	//need to do checks first
	printf("cwnd_addPkt: nextMss=%d\n", cwnd_nextMss(cwnd));
	//then update variables
	//if there are spots in the congestion window
	if(cwnd->next_seq - cwnd->last_seq < C_WND)
	{
		memcpy(cwnd->packets[cwnd_nextMss(cwnd)], buf, PACKET_SIZE);
		cwnd->next_seq = (cwnd->next_seq+PACKET_SIZE) % cwnd->size;
		printf("cwnd_addPkt: returning\n");
		return true;
	}
	printf("cwnd_addPkt: window full!\n");
	return false;
}

int cwnd_nextMsgIndex(cwnd_t *cwnd)
{
	int i = cwnd_lastMss(cwnd);
	do{
		if(cwnd->acks[i] == 0)
		{
			printf("cwnd_nextMsgIndex: index=%d\n",i);
			return i;
		}
		i = (i+1)%((C_WND*2)/PACKET_SIZE);
	}while(i != cwnd_lastMss(cwnd));
	return -1;
}

uint8_t cwnd_nextMsgLen(cwnd_t *cwnd)
{
	int i = cwnd_nextMsgIndex(cwnd);
	printf("cwnd_nextMsgLen: index=%d\n",i);
	if(i >= 0)
	{
		printf("cwnd_nextMsgLen: size=%d\n",getSize(cwnd->packets[i]));
		return getSize(cwnd->packets[i]);
	}
	else
	{
		return 0;
	}
		//update the sequence number
}
char * cwnd_nextMsg(cwnd_t *cwnd)
{
	int i = cwnd_nextMsgIndex(cwnd);

	if(i >= 0)
	{
		printf("cwnd_nextMsg: msg=%s\n",getBody(cwnd->packets[0]));
		return getBody(cwnd->packets[i]);
	}
	else
	{
		return NULL;
	}
}

void cwnd_markNextMsgRead(cwnd_t *cwnd)
{
	int i = cwnd_nextMsgIndex(cwnd);
	if(i >= 0)
	{
		cwnd->acks[i] = true;
	}
}

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
	// printf("setACKNum: index=%d\n",index);
}

bool getACK(const char * pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[ACK_OFFSET>>5];
	// printf("getACK: temp=%x\n",temp);
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

	// printf("setACK: ack=%x\n",((uint32_t *)pkt)[ACK_OFFSET>>5] );
}	


bool getLast(const char * pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[LAST_OFFSET>>5];
	// printf("getLast: temp=%x\n",temp);
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

	// printf("setLast: last=%x\n",((uint32_t *)pkt)[LAST_OFFSET>>5] );
}

bool getShake(const char * pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[SHAKE_OFFSET>>5];
	// printf("getLast: temp=%x\n",temp);
	temp = temp >> 29;
	temp = temp & 0x1;
	return temp;
}

void setShake(char * pkt, bool last)
{
		// int offset = ACK_OFFSET;
	uint32_t temp = last;

	temp = temp << 29;

	((uint32_t *)pkt)[SHAKE_OFFSET>>5] = (((uint32_t *)pkt)[SHAKE_OFFSET>>5]&0xDFFFFFFF) | temp;

	// printf("setLast: last=%x\n",((uint32_t *)pkt)[LAST_OFFSET>>5] );
}

uint16_t getSize(const char * pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[SIZE_OFFSET>>5];
	// printf("getSize: temp=%x\n",temp);
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

	// printf("setSize: size=%x\n",((uint32_t *)pkt)[SIZE_OFFSET>>5] );
}

uint16_t getChecksum(const char *pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[CHECKSUM_OFFSET>>5];
	// printf("getChecksum: temp=%x\n",temp);
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

char * generatePacket(char * pkt,
					   uint32_t seq_num, 
					   uint32_t ack_num, 
					   bool ack, 
					   bool last,
					   bool shake,
					   char * buff,
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
	setSize(pkt, count);
	setBody(pkt, buff, count);
	cs = checksum(pkt, PACKET_SIZE);
	setChecksum(pkt, cs);

	//finish
	return pkt;
	}
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

void freePacket(char * pkt)
{
	free(pkt);
	pkt = NULL;
}

//need to support binary files by using memcpy instead of strncpy
/*
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
*/

//takes care of timeout
int writePacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndW)
{
	int ret = -1;
	char buf[PACKET_SIZE];
	memset(buf, 0, PACKET_SIZE);

	memcpy(buf, cwndW->packets[cwnd_lastMss(cwndW)], PACKET_SIZE);
	if (sendto(sockfd, buf, PACKET_SIZE, 0, sockaddr, socklen)<0) {
		error("sendto");
		ret = -1;
	}
	else{
		/*
		//wait for ACK
		do{
			bytesrecv = recvfrom(sockfd, buf, PACKET_SIZE, 0, (struct sockaddr *)&sockaddr, &socklen);
		}while(bytesrecv <= 0)//maybe it should be <=PACKET_SIZE
		if(bytesrecv == PACKET_SIZE)
		{
			if( getLAST(buff)== true && 
				getACK(buf) == true && 
				getShake(buf) == true)
			{
				ret = 0;
			}
			else{
				printf("writePacket: issues with ACK");
				ret = -1;
			}
		}
		else{
			//the entire packet was not received
			printf("writePacket: entire packet not received\n");
			ret = -1;
		}
		*/
		printf("writePacket: packet sent!\n");
		ret = 0;
	}
	return ret;
}


//should take care of piecing together the body of the file
int readPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndR)
{
	char buf[PACKET_SIZE];
	int bytesrecv = 0;

	//would like to assume msgBody is a null pointer

	do{
		bytesrecv = recvfrom(sockfd, buf, PACKET_SIZE, 0, sockaddr, &socklen);
	}while(bytesrecv <= 0);
	/*
	//if it is in the range of being read
	if(cwnd_addPkt(cwndR, getSeqNum(buf)) == true)
	{
		//need lower level function to handle multiple packets if there is a long filename
		printPacket(pkt);
		msgBody = getBody(pkt);
		//todo look up what the ACK number is
		
	}
	*/
	cwnd_addPkt(cwndR, buf);

	printPacket(buf);
	printf("received message: %s\n", getBody(buf));
	return PACKET_SIZE;
}

bool readAckPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndW)
{
	char buf[PACKET_SIZE];
	int bytesrecv = 0;
	bool ret = false;

	do{
		bytesrecv = recvfrom(sockfd, buf, PACKET_SIZE, 0, sockaddr, &socklen);
	}while(bytesrecv <= 0);//maybe it should be <=PACKET_SIZE
		if(bytesrecv == PACKET_SIZE)
		{
			if(getACK(buf) == true)
			{
				/*
				if(cwnd_checkIn(cwndW, getSeqNum(buf)) == true)
				{
					//attempt to set the ack
					//this hopefully supports both GoBackN & SelectiveRepeat
					//GoBackN will only return true if it is the next to be
					if(cwnd_addAck(cwndW, getSeqNum))
					{
						return true;
					}
					else{
						return false;
					}
				}
				ret = 0;
				*/
				printf("readAckPacket: ack received! but cwnd not modified\n");
				printPacket(buf);
				ret = true;
			}
			else{
				printf("writePacket: issues with ACK");
				ret = false;
			}
		}
		else{
			//the entire packet was not received
			printf("writePacket: entire packet not received\n");
			ret = false;
		}
	//if the sequence number is one that has not been ack'ed
	
	return ret;
}

bool writeAckPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndR)
{
	char buf[PACKET_SIZE],pkt[PACKET_SIZE];
	int lastMss = cwnd_lastMss(cwndR);

	if(cwnd_lastMss(cwndR) == cwnd_nextMss(cwndR))
	{
		printf("writeAckPacket: no packets unacknowledged\n");
		return false;
	}
	printf("writeAckPacket: lastMss=%d\n",lastMss);
	memcpy(buf,cwndR->packets[lastMss],PACKET_SIZE);
	printf("writeAckPacket: after memset\n");
	generatePacket(pkt, getSeqNum(buf),getACKNum(buf),1,1,0,"",0);
	if (sendto(sockfd, pkt, PACKET_SIZE, 0, sockaddr, socklen)<0) 
	{
		error("sendto");
		return false;
	}
	return true;
}
/*
int connectTCP(int sockfd, struct sockaddr_in sockaddr, socklen_t socklen)
{
	char pkt[PACKET_SIZE];
	int bytessent = 0, ret = -1;

	pkt= generatePacket(pkt,0,0,0,1,1,"", 0);
	bytessent = writePacket(sockfd, (struct sockaddr *)&sockaddr, socklen, pkt);
	if(bytesrecv > 0) {
		
		//need lower level function to handle multiple packets if there is a long filename
		printPacket(packet); //debugging
		ret = 0;
	}
	else{
		printf("Error connecting!\n");
		ret = -1;
	}
	// echo file request

	return ret;
}

int acceptTCP(int sockfd, struct sockaddr_in sockaddr, socklen_t socklen)
{
	char *msgBody = NULL;
	int bytesrecv = 0;
	int ret = -1;

	bytesrecv = readPackets(sockfd, sockaddr, socklen, msgBody);
	if(bytesrecv == 0)
	{
		//fantastic!
		ret = 0;
	}
	else if(bytesrecv > 0)
	{
		//I don't know, 
		//maybe the client sends a request along with the first connection request
		if (msgBody != NULL)
		{
			freePacket(packet);
		}
		ret = 0;
	}
	else{
		ret = bytesrecv;
	}

	// echo file request
	printf("connection requested\n");

	return ret;
}
*/
// INPUT: 
// OUTPUT: 
// RETURNS: number of bytes written
// ABOUT: the main function for transfering files
//			uses handshake and transfer functions
//        creates the contention window. 
int writeTCP(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, char * buf, size_t nbytes)
{
	char *pkt;//for testing
	cwnd_t *cwndR;
    cwnd_t *cwndW;

    pkt = malloc(PACKET_SIZE);
    cwndR = cwnd_init(cwndR);
    cwndW = cwnd_init(cwndW);
	//read file

	//break up file into packets

	//write packets 
	pkt = generatePacket(pkt,cwndW->next_seq,cwndW->next_seq,0,1,0,buf,nbytes);
	cwnd_addPkt(cwndW, pkt);
   	free(pkt);
   	writePacket(sockfd, sockaddr, socklen, cwndW);
    readAckPacket(sockfd, sockaddr, socklen, cwndW);

    cwnd_free(cwndR);
	cwnd_free(cwndW);
	return nbytes;//change if there are bytes unwritten.
}

// INPUT: 
// OUTPUT: 
// RETURNS: number of bytes read in the message body
// ABOUT: the main function for reading TCP118 packets
//		 probably should use congestion window, but not sure why
int readTCP(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, char * msgBody)
{
	int bytesrecv = 0, msglen = 0;
	char msgBuf[MAX_BODY_SIZE];
	cwnd_t *cwndR;
    cwnd_t *cwndW;

    cwndR = cwnd_init(cwndR);
    cwndW = cwnd_init(cwndW);


	if(msgBody != NULL)
	{
		free(msgBody);
		msgBody = NULL;
	}
	msgBody = malloc(MAX_BODY_SIZE);

	do{
		printf("readTCP: start do\n");
		uint8_t thisMsgLen = 0;
		memset(msgBuf, 0, MAX_BODY_SIZE);
		printf("readTCP: before readPacket\n");
		bytesrecv = readPacket(sockfd, sockaddr, socklen, cwndR);
		//writeAck should work even if nothing was received
		printf("readTCP: before readPacket\n");
		if(bytesrecv > 0)
		{
			thisMsgLen += cwnd_nextMsgLen(cwndR);
			printf("readTCP: msglen=%d\n", thisMsgLen);
			memcpy(msgBuf, cwnd_nextMsg(cwndR),thisMsgLen);
			cwnd_markNextMsgRead(cwndR);
			writeAckPacket(sockfd, sockaddr, socklen, cwndR);
			msgBody = realloc(msgBody, msglen + thisMsgLen);
			memcpy(&(msgBody[msglen]), msgBuf, msglen);
			msglen += thisMsgLen;
			if(thisMsgLen < MAX_BODY_SIZE) break;//temp fix to waiting read
		}
		printf("readTCP: bytesrecv=%d\n",bytesrecv);
	}while(bytesrecv != 0);
	printf("readTCP: appended all of file\n");
	printf("readTCP: msg=%s\n",msgBody);
    cwnd_free(cwndR);
	cwnd_free(cwndW);
	return msglen;
}


// INPUT: 
// OUTPUT: 
// RETURNS: number of bytes written
// ABOUT: the main function for transfering files
//			uses handshake and transfer functions

