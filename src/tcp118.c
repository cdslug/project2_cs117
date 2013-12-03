#include "tcp118.h"
#include "probability.h"


//takes care of timeout
//returns number of packets remaining
int writePackets(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndW, double p_loss, double p_corr)
{
	byte_t buf[PACKET_SIZE];
	byte_t *pkt = NULL;
	int i = 0, iter = 0;;

	// clear packet
	memset(buf, 0, PACKET_SIZE);

	//iterate through all of the unacknowledged packets
	printf("writePacket: pending Acks=%d\n",cwnd_numPendingAcks(cwndW));
	cwnd_print(cwndW);

	for(iter = 0, i = cwnd_lastPendingAckMss(cwndW); iter < cwnd_numPendingAcks(cwndW); iter ++,i++)
	{
		if(cwnd_getAck(cwndW, i*PACKET_SIZE) == 1)
		{
			//already acknowledged
			continue;
		}
		printf("writePacket: sending index=%d\n", i);
		if(p_check(p_corr) && false){
			memset(buf, 0, PACKET_SIZE);
			printf("writePacket: CORRUPT! seq=%d\n",i*PACKET_SIZE);
		}
		else{
			pkt = cwndW->packets[i];

			if(pkt == NULL)
			{
				printf("writePacket: ERROR, packet is NULL");
			}
			memcpy(buf, pkt, PACKET_SIZE);
		}
		printf("writePacket: about to send, packet=\n");
		printPacket(buf);

		if(!p_check(p_loss))
		{
			if (sendto(sockfd, buf, PACKET_SIZE, 0, sockaddr, socklen)<0) {
				error("sendto");
			}
		}
		else
		{
			printf("writePacket: LOST! seq=%d\n",i*PACKET_SIZE);
		}
	}
	
	printf("writePacket: exit\n");
	return 0;
}


//should take care of piecing together the body of the file
int readPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndR)
{
	byte_t buf[PACKET_SIZE];
	int bytesrecv = 0;
	uint32_t expSeq = 0;

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
	expSeq = cwnd_lastPendingAckMss(cwndR)*PACKET_SIZE;
	if(expSeq == -1*PACKET_SIZE)
	{
		expSeq = cwnd_nextSeq(cwndR);
	}
	if(checksum(buf, PACKET_SIZE) == 0 && getSeqNum(buf) == expSeq)
	{
		//extracting is done in readTCP
		//deliver_data is done in readTCP
		//ack packet is sent in readTCP using writeAckPacket
		//expectedSequenceNumber is updated in cwnd_addPkt
		cwnd_addPkt(cwndR, buf);
	}
	else
	{
		printf("readPacket: checksum or order failed!\n");
		printf("readPacket: checksum=%d, recSeq=%d,expSeq=%d\n", checksum(buf, PACKET_SIZE), getSeqNum(buf), expSeq);
	}
	printf("readPacket: printPacket=\n");
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
	printf("readAckPacket: enter\n");
	if(cwnd_numPendingAcks(cwndW) == 0)
	{
		printf("readAckPacket: no pending acks\n");
		ret = false;
	}
	else
	{
		printf("readAckPacket: about to read Ack\n");
		do{
			bytesrecv = recvfrom(sockfd, buf, PACKET_SIZE, 0, sockaddr, &socklen);
		}while(bytesrecv <= 0);
		printf("readAckPacket: bytesrecv=%d, should=%d\n",bytesrecv, PACKET_SIZE);
		// printPacket(buf);
		printf("readAckPacket: getAck=%d, checksum=%d\n",getACK(buf),checksum(buf, PACKET_SIZE));
		if(bytesrecv == PACKET_SIZE)
		{
			if(getACK(buf) == true && checksum(buf, PACKET_SIZE) == 0)
			{
				printf("readAckPacket: pktSeq=%d\n, lastSeq=%d\n",getACKNum(buf), cwnd_lastPendingAckMss(cwndW)*PACKET_SIZE);
				//uint32_t expSeq = cwnd_lastPendingAckMss(cwndW)*PACKET_SIZE;
				//if(expSeq == -1*PACKET_SIZE)
				//let there be an error because it was already ack'd
				if(cwnd_checkIn(cwndW, getACKNum(buf)) == true){
					//cwnd_setAck(cwndW, getACKNum(buf));
					cwnd_setAllPrevAck(cwndW, getACKNum(buf));
					cwnd_print(cwndW);
					printf("readAckPacket: in order ack received!\n");
					printPacket(buf);
					ret = true;
				}
				else
				{
					printf("readAckPacket:out of order ack received.\n");
					ret = false;
				}
				
			}
			else{
				//if ack is corrupt, it will be unacknowledged. 
				//what happens to the timer?
				printf("readAckPacket: issues with ACK\n");
				ret = false;
			}
		}
		else{
			//the entire packet was not received
			printf("readAckPacket: entire packet not received\n");
			ret = false;
		}
	//if the sequence number is one that has not been ack'ed
	}
	return ret;
}

bool writeAckPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndR, double p_loss, double p_corr)
{
	int ret = false;
	byte_t buf[PACKET_SIZE],pkt[PACKET_SIZE];
	uint32_t lastSeq = cwnd_lastPendingAckMss(cwndR) * PACKET_SIZE;

	if(lastSeq == -1*PACKET_SIZE)
	{
		lastSeq = cwnd_nextSeq(cwndR)-PACKET_SIZE;
	}
	
	if(cwnd_numPendingAcks(cwndR) == 0)
	{
		printf("writeAckPacket: no packets unacknowledged\n");
		generatePacket(pkt, lastSeq, lastSeq,1,1,0,"",0);
		if (sendto(sockfd, pkt, PACKET_SIZE, 0, sockaddr, socklen)<0) 
		{
			error("sendto");
			ret = false;
		}
		ret = false;
	}
	else
	{
		printf("writeAckPacket: lastSeq=%d\n",lastSeq);
			
		cwnd_print(cwndR);
		
		// memcpy(buf, (byte_t *)cwnd_getPktSeq(cwndR, lastSeq),PACKET_SIZE);
		
		// printf("writeAckPacket: after memcpy\n");
		generatePacket(pkt, lastSeq,lastSeq,1,1,0,"",0);
		if(p_check(p_corr)) // corrupt packet
		{
			memset(pkt, 0, PACKET_SIZE);
			printf("writeAckPacket: CORRUPT! seq=%d",lastSeq);
		}

		printf("writeAckPacket: sending ack\n");
		// printPacket(pkt);
		if(!p_check(p_loss))
		{
			if (sendto(sockfd, pkt, PACKET_SIZE, 0, sockaddr, socklen)<0) 
			{
				error("sendto");
				return false;
			}
		}
		else
		{
			printf("writeAckPacket: LOST! seq=%d",lastSeq);
		}	
		cwnd_setAllPrevAck(cwndR, lastSeq);
		ret = true;
	}
	return ret;
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
	clock_t start;
	int i = 0;
	byte_t **pkts = NULL;//for testing
	// cwnd_t *cwndR;
    cwnd_t *cwndW;

    // cwndR = cwnd_init(cwndR);
    cwndW = cwnd_init(cwndW);
    seed(); // seed random for packet loss and corruption

	printf("writeTCP: bufToPackets\n");
	//break up buf into packets
	pkts = bufToPackets(buf, nbytes);

	//write packets 
	i = 0;
	while(cwnd_numPendingAcks(cwndW) > 0 || pkts[i] != 0)
	{
		// printPacket(pkts[i]);
		while(pkts[i] != 0 && cwnd_checkAdd(cwndW))
		{
			cwnd_addPkt(cwndW, pkts[i]);
			i++;
		}

		if(cwnd_numReceivedAcks(cwndW) == cwnd_maxMss(cwndW)/2 && 
		   cwnd_checkAdd(cwndW) == false)
		{
			printf("readTCP: All acks received, shifting window\n");
			cwnd_shiftWnd(cwndW);
			//start timer
			start = clock();
			//maybe add the reset timer in here
		}

		if((float)(clock()-start)/CLOCKS_PER_SEC > 0.160) // if the timer exceeds 160 ms, timeout
			//timeout stuff goes here
		{
			int i = 0; //blank code
		}
			
		//p_check(p_loss) should actually be in write packet
		//write packet goes through all unack'd packets
		//choosing to skip it here will skip writing all packets
		

		
		writePackets(sockfd, sockaddr, socklen, cwndW, p_loss, p_corr);
    	
    	while(readAckPacket(sockfd, sockaddr, socklen, cwndW) == true);
		
	}
   	freePackets(pkts);
   	
    // cwnd_free(cwndR);
	cwnd_free(cwndW);
	return nbytes;//change if there are bytes unwritten.
}

// INPUT: 
// OUTPUT: 
// RETURNS: number of bytes read in the message body
// ABOUT: the main function for reading TCP118 packets
//		 probably should use congestion window, but not sure why
int readTCP(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, byte_t ** msgBodyPtr, double p_loss, double p_corr)
{
	
	int bytesrecv = 0, msgLen = 0;
	byte_t *pkt = NULL;
	byte_t buf[PACKET_SIZE];
	byte_t * msgBody =NULL;
	cwnd_t *cwndR;
    // cwnd_t *cwndW;

    cwndR = cwnd_init(cwndR);
    // cwndW = cwnd_init(cwndW);

printf("readTCP: enter\n");
	if(msgBody != NULL)
	{
		free(msgBody);
		msgBody = NULL;
	}
	msgBody = malloc(MAX_BODY_SIZE);
	memset(msgBody, 0, MAX_BODY_SIZE);
	do{
		printf("readTCP: start do\n");
		uint32_t currLen = 0;
		
		//if all packets are ACK'd and 
		if(cwnd_numReceivedAcks(cwndR) == cwnd_maxMss(cwndR)/2 && 
		   cwnd_checkAdd(cwndR) == false)
		{
			printf("readTCP: All acks received, shifting window\n");
			cwnd_shiftWnd(cwndR);
		}
		// printf("readTCP: before readPacket\n");

		//getLast returns false when pkt == NULL
		if(getLast(pkt) == false)
		{
			bytesrecv = readPacket(sockfd, sockaddr, socklen, cwndR);
		}
		//writeAck should work even if nothing was received
		printf("readTCP: after readPacket\n");
		cwnd_print(cwndR);
		printf("readTCP: no segfault yet\n");
		if(cwnd_lastPendingAckMss(cwndR) != -1)
		{
			uint32_t nextSeq = cwnd_lastPendingAckMss(cwndR)*PACKET_SIZE;
			if(nextSeq == -1*PACKET_SIZE)
			{
				nextSeq = cwnd_nextSeq(cwndR)-PACKET_SIZE;
			}
			pkt = (byte_t *)cwnd_getPktSeq(cwndR, nextSeq);
		}
		else{
			pkt = NULL;
		}
		if(pkt != NULL)
		{
			currLen = getSize(pkt);
			printf("readTCP: currLen=%d, msgLen=%d\n", currLen, currLen + msgLen );
			printf("readTCP: currMsg=%s\n",getBody(pkt));
			//ack will mark the last packet read
			//I don't know what to do if this ack is lost
			//if there is one packet, readTCP will exit
			//the sender will try to resend the packet
			//   but the server will no longer be looking for acks

			writeAckPacket(sockfd, sockaddr, socklen, cwndR, p_loss, p_corr);
			msgBody = realloc(msgBody, msgLen + currLen);
			printf("readTCP: realloc msg=%s\n",msgBody);
			memcpy(&(msgBody[msgLen]), getBody(pkt), currLen);
			msgLen += currLen;
			//if(currLen < MAX_BODY_SIZE) break;//temp fix to waiting read
		}
		printf("readTCP: bytesrecv=%d\n",bytesrecv);
	}while(getLast(pkt) == false && 
		   cwnd_numPendingAcks(cwndR) == 0);
	printf("readTCP: appended all of file, msgLen=%d\n", msgLen);
	printf("readTCP: msg=%s\n",msgBody);
    cwnd_free(cwndR);
	// cwnd_free(cwndW);
	*msgBodyPtr = msgBody;
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

int writeFile(char * fileName, char * fileBuf, int bytes){
	char ch;
	FILE *fp;
	size_t n;

	assert(fileBuf!=NULL);

	fp = fopen(fileName, "w");
	if(fp == NULL){
		return -1; // return -1 for error state
	}

	n = fwrite(fileBuf, 1, bytes, fp);

	fclose(fp);

	return (int) n;
}
