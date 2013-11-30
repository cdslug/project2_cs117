#include "tcp118.h"
#include "probability.h"

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
	cwnd->packets = (byte_t **)malloc(sizeof(byte_t*)*(C_WND * 2));

	for(i = 0; i < (C_WND * 2)/PACKET_SIZE; i++)
	{
		cwnd->packets[i] = malloc(sizeof(byte_t)*PACKET_SIZE);
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

uint32_t cwnd_numPendingAcks(cwnd_t *cwnd)
{
	uint32_t num = ((uint32_t)cwnd_nextMss(cwnd)-(uint32_t)cwnd_lastMss(cwnd)) % (cwnd->size/PACKET_SIZE);
	printf("cwnd_numPendingAcks: nextMss=%d, lastMss=%d\n",cwnd_nextMss(cwnd), cwnd_lastMss(cwnd));
	printf("cwnd_numPendingAcks: num=%d\n",num);
	return num;
}

void cwnd_setAck(cwnd_t *cwnd, uint32_t seqNum)
{
	int i = seqNum/PACKET_SIZE;
	if(cwnd_checkIn(cwnd, seqNum))
	{
		cwnd->acks[(seqNum%cwnd->size)/PACKET_SIZE] = 1;
	}
	else
	{
		printf("cwnd_setAck: seqNum out of frame");
	}
}
bool cwnd_getAck(cwnd_t *cwnd, uint32_t seqNum)
{
	if(cwnd->acks[(seqNum%cwnd->size)/PACKET_SIZE] == 0)
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
	if(seqNum >= cwnd->last_seq && seqNum < cwnd->next_seq)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool cwnd_checkAdd(cwnd_t *cwnd)
{
	if((cwnd->next_seq - cwnd->last_seq) % cwnd->size >= C_WND){
		return false;
	}
	else{
		return true;
	}
}

bool cwnd_addPkt(cwnd_t *cwnd, byte_t *buf)
{
	//need to do checks first
	printf("cwnd_addPkt: nextMss=%d\n", cwnd_nextMss(cwnd));
	//then update variables
	//if there are spots in the congestion window
	if(cwnd_checkAdd(cwnd))
	{
		// printf("cwnd_addPkt: adding\n");
		memcpy(cwnd->packets[cwnd_nextMss(cwnd)], buf, PACKET_SIZE);
		cwnd->acks[cwnd->next_seq/PACKET_SIZE] = false;
		cwnd->next_seq = (cwnd->next_seq+PACKET_SIZE) % cwnd->size;
		return true;
	}
	else{
		printf("cwnd_addPkt: window full!\n");
		return false;
	}
}

int cwnd_lastPktIndex(cwnd_t *cwnd)
{
	int i = cwnd_lastMss(cwnd);
	do{
		if(cwnd->acks[i] == 0)
		{
			printf("cwnd_lastPktIndex: index=%d\n",i);
			return i;
		}
		i = (i+1)%((C_WND*2)/PACKET_SIZE);
	}while(i != cwnd_lastMss(cwnd));
	return -1;
}

byte_t * cwnd_getLastPkt(cwnd_t *cwnd)
{
	int i = cwnd_lastPktIndex(cwnd);

	if(i >= 0)
	{
		printf("cwnd_getLastPkt: msg=%s\n",getBody(cwnd->packets[0]));
		return cwnd->packets[i];
	}
	else
	{
		return NULL;
	}
}

void cwnd_markLastPktRead(cwnd_t *cwnd)
{
	int i = cwnd_lastPktIndex(cwnd);
	printf("cwnd_markLastPktRead: index=%d\n",i);
	if(i >= 0)
	{
		cwnd->acks[i] = 1;
		cwnd->last_seq = (cwnd->last_seq+PACKET_SIZE)%cwnd->size;
	}
	printf("cwnd_markLastPktRead: state=%d\n", cwnd_getAck(cwnd,i*PACKET_SIZE));
}

void cwnd_print(cwnd_t *cwnd)
{
	int i = 0;
	printf("cwnd_print: START\n");
	printf("\tc_wnd=%d\n",cwnd->c_wnd);
	printf("\tsize=%d\n",cwnd->size);
	printf("\tlast_seq=%d\n",cwnd->last_seq);
	printf("\tnext_seq=%d\n",cwnd->next_seq);
	printf("\tack:\n");
	for(i = 0; i < cwnd->size/PACKET_SIZE; i++)
	{
		printf("\t\t#%d=%d\n",i,cwnd_getAck(cwnd,(i*PACKET_SIZE)));
	}
	printf("cwnd_print: END\n");
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

uint32_t getSeqNum(const byte_t * pkt)
{
	return ((uint32_t *)pkt)[SEQ_NUM_OFFSET>>5];
}

void setSeqNum(byte_t * pkt, uint32_t seqNum)
{
	((uint32_t *)pkt)[SEQ_NUM_OFFSET>>5] = seqNum;
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
		return true;
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

bool getShake(const byte_t * pkt)
{
	uint32_t temp = ((uint32_t *)pkt)[SHAKE_OFFSET>>5];
	// printf("getLast: temp=%x\n",temp);
	temp = temp >> 29;
	temp = temp & 0x1;
	return temp;
}

void setShake(byte_t * pkt, bool last)
{
		// int offset = ACK_OFFSET;
	uint32_t temp = last;

	temp = temp << 29;

	((uint32_t *)pkt)[SHAKE_OFFSET>>5] = (((uint32_t *)pkt)[SHAKE_OFFSET>>5]&0xDFFFFFFF) | temp;

	// printf("setLast: last=%x\n",((uint32_t *)pkt)[LAST_OFFSET>>5] );
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
	printf("setSize: start size=%d\n",size);
	uint32_t temp = size;
	int ret = size;
	if(size > MAX_BODY_SIZE){
		temp = MAX_BODY_SIZE;
		ret = MAX_BODY_SIZE;
	}

	temp = temp << 16;
	printf("setSize: temp=%x\n",temp);
	((uint32_t *)pkt)[SIZE_OFFSET>>5] = (((uint32_t *)pkt)[SIZE_OFFSET>>5]&0xFC00FFFF) | temp;
	printf("setSize: size=%d\n",getSize(pkt));
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
					   bool shake,
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

byte_t** bufToPackets(byte_t * buf, uint32_t nbytes)
{
	int numPackets = 0;
	int i = 0;
	byte_t **packetArray = NULL;

	if(buf == NULL){
		return NULL; //maybe I should return an allocated byte_t**
	}


	numPackets = (nbytes+(MAX_BODY_SIZE-1))/MAX_BODY_SIZE; //rounds up
	printf("bufToPackets, number of packets=%d\n",numPackets);
	//allocate space
	printf("bufToPackets, before packetarray malloc\n");
	packetArray = (byte_t**)malloc(sizeof(byte_t*) * (numPackets+1));
	memset(packetArray, 0, sizeof(byte_t*)*(numPackets+1));
	for(i = 0; i < numPackets; i++){
		printf("bufToPackets, allocating string %d\n", i);
		packetArray[i] = (byte_t*)malloc(sizeof(byte_t) * (PACKET_SIZE));
		memset(packetArray[i], '\0', sizeof(byte_t)*(PACKET_SIZE));
	}

	//divide up the file string
	for(i = 0; i < numPackets; i++){
		int pieceLen = MAX_BODY_SIZE;
		printf("bufToPackets, copy to pkt#=%d, nbytes=%d, pieceLen=%d\n",i, nbytes, pieceLen);
		if(nbytes < MAX_BODY_SIZE)
		{
			pieceLen = nbytes;
			nbytes = 0;
			generatePacket(packetArray[i], 0,0,0,1,0,&(buf[i*MAX_BODY_SIZE]), pieceLen);
		}
		else{
			nbytes -= MAX_BODY_SIZE;
			generatePacket(packetArray[i], 0,0,0,0,0,&(buf[i*MAX_BODY_SIZE]), pieceLen);
		}		
	}

	return packetArray;
}


//takes care of timeout
//returns number of packets remaining
int writePacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndW, double p_corr)
{
	byte_t buf[PACKET_SIZE];
	byte_t *pkt = NULL;
	int i = 0;

	memset(buf, 0, PACKET_SIZE);

	

	//iterate though all of the unacknowledged packets
	for(i = cwnd_lastMss(cwndW); i < cwnd_numPendingAcks(cwndW); i++)
	{
		
		if(p_check(p_corr)){
			memset(buf, 0, PACKET_SIZE);
		}
		else{
			pkt = cwndW->packets[i];
			if(pkt == NULL)
			{
				printf("writePacket: ERROR, packet is NULL");
			}
			memcpy(buf, pkt, PACKET_SIZE);
		}

		if (sendto(sockfd, buf, PACKET_SIZE, 0, sockaddr, socklen)<0) {
			error("sendto");
		}
	}
	
	printf("writePacket: packet sent!\n");
	return 0;
}


//should take care of piecing together the body of the file
int readPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndR)
{
	byte_t buf[PACKET_SIZE];
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
	// printf("readPacket: received message=%s\n", getBody(buf));
	return PACKET_SIZE;
}

//TODO: need to take into account acks that are behind
bool readAckPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndW)
{
	byte_t buf[PACKET_SIZE];
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
				cwnd_setAck(cwndW, getSeqNum(buf));
				printf("readAckPacket: ack received! but cwnd not modified\n");
				printPacket(buf);
				ret = true;
			}
			else{
				printf("readAckPacket: issues with ACK");
				ret = false;
			}
		}
		else{
			//the entire packet was not received
			printf("readAckPacket: entire packet not received\n");
			ret = false;
		}
	//if the sequence number is one that has not been ack'ed
	
	return ret;
}

bool writeAckPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndR, double p_corr)
{
	byte_t buf[PACKET_SIZE],pkt[PACKET_SIZE];
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
	cwnd_markLastPktRead(cwndR);
	return true;
}
/*
int connectTCP(int sockfd, struct sockaddr_in sockaddr, socklen_t socklen)
{
	byte_t pkt[PACKET_SIZE];
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
	byte_t *msgBody = NULL;
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
int writeTCP(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, byte_t * buf, size_t nbytes, double p_loss, double p_corr)
{
	int i = 0;
	byte_t **pkts = NULL;//for testing
	cwnd_t *cwndR;
    cwnd_t *cwndW;

    cwndR = cwnd_init(cwndR);
    cwndW = cwnd_init(cwndW);
    seed(); // seed random for packet loss and corruption

/*
	//break up buf into packets
	pkts = bufToPackets(buf, nbytes);
	cwnd_addPkt(cwndW, pkts[0]); //0
	cwnd_markLastPktRead(cwndW);
	cwnd_addPkt(cwndW, pkts[0]); //1
	cwnd_markLastPktRead(cwndW);
	cwnd_addPkt(cwndW, pkts[0]); //2
	cwnd_markLastPktRead(cwndW);
	cwnd_addPkt(cwndW, pkts[0]); //3
	cwnd_markLastPktRead(cwndW);

	// cwnd_print(cwndW);

	cwnd_addPkt(cwndW, pkts[0]); //4
	cwnd_markLastPktRead(cwndW);
	cwnd_addPkt(cwndW, pkts[0]); //5
	cwnd_markLastPktRead(cwndW);
	cwnd_addPkt(cwndW, pkts[0]); //6
	// cwnd_markLastPktRead(cwndW);
	cwnd_addPkt(cwndW, pkts[0]); //7
	// cwnd_markLastPktRead(cwndW);
	cwnd_addPkt(cwndW, pkts[0]); //0
	// cwnd_markLastPktRead(cwndW);
	cwnd_addPkt(cwndW, pkts[0]); //1
	// cwnd_markLastPktRead(cwndW);
	cwnd_addPkt(cwndW, pkts[0]); //2
	// cwnd_markLastPktRead(cwndW);

	printf("writeTCP: ACKs left=%d\n", cwnd_numPendingAcks(cwndW));
	cwnd_print(cwndW);
	*/

	//write packets 
	while(cwnd_numPendingAcks(cwndW) > 0)
	{
		if(pkts[i] != 0 && cwnd_checkAdd(cwndW))
		{
			cwnd_addPkt(cwndW, pkts[i]);
			i++;
		}

		if(!p_check(p_loss))
			writePacket(sockfd, sockaddr, socklen, cwndW, p_corr);
    	while(readAckPacket(sockfd, sockaddr, socklen, cwndW) == true);
		
	}
   	freePackets(pkts);
   	
    cwnd_free(cwndR);
	cwnd_free(cwndW);
	return nbytes;//change if there are bytes unwritten.
}

// INPUT: 
// OUTPUT: 
// RETURNS: number of bytes read in the message body
// ABOUT: the main function for reading TCP118 packets
//		 probably should use congestion window, but not sure why
int readTCP(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, byte_t * msgBody, double p_loss, double p_corr)
{
	int bytesrecv = 0, msgLen = 0;
	byte_t *pkt = NULL;

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
	memset(msgBody, 0, MAX_BODY_SIZE);
	do{
		// printf("readTCP: start do\n");
		uint8_t currLen = 0;
		
		// printf("readTCP: before readPacket\n");
		bytesrecv = readPacket(sockfd, sockaddr, socklen, cwndR);
		//writeAck should work even if nothing was received
		// printf("readTCP: after readPacket\n");
		pkt = cwnd_getLastPkt(cwndR);
		if(pkt != NULL)
		{
			currLen += getSize(pkt);
			printf("readTCP: currLen=%d\n", currLen);
			printf("readTCP: currMsg=%s\n",getBody(pkt));
			//ack will mark the last packet read
			writeAckPacket(sockfd, sockaddr, socklen, cwndR, p_corr);
			msgBody = realloc(msgBody, msgLen + currLen);
			memcpy(&(msgBody[msgLen]), getBody(pkt), currLen);
			msgLen += currLen;
			//if(currLen < MAX_BODY_SIZE) break;//temp fix to waiting read
		}
		printf("readTCP: bytesrecv=%d\n",bytesrecv);
	}while(getLast(pkt) == false);
	printf("readTCP: appended all of file\n");
	printf("readTCP: msg=%s\n",msgBody);
    cwnd_free(cwndR);
	cwnd_free(cwndW);
	return msgLen;
}


// INPUT: 
// OUTPUT: 
// RETURNS: number of bytes written
// ABOUT: the main function for transfering files
//			uses handshake and transfer functions


int readFile(char * fileName, char * fileBuf, int BUFLEN){
        char ch;
        FILE *fp;

        assert(fileBuf!=NULL);
        memset(fileBuf, 0, BUFLEN); // clear fileContent buffer

        fp = fopen(fileName, "r"); // open the file in read mode

        if(fp == NULL){ 
                return -1; //return -1 for error state
        }

        int n = 0;
        while((ch = fgetc(fp)) != EOF ){
                assert(n < BUFLEN);
                fileBuf[n++] = ch;
        }

        fclose(fp);
        return n;
}
