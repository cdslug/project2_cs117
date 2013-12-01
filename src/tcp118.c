#include "tcp118.h"
#include "probability.h"


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
		printf("writePacket: sending index=%d\n", i);
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
	
	printf("writePacket: exit\n");
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
	if(checksum(buf, PACKET_SIZE) == 0 &&
	   getSeqNum(buf) == cwnd_lastSeq(cwndR))
	{
		//extracting is done in readTCP
		//deliver_data is done in readTCP
		//ack packet is sent in readTCP using writeAckPacket
		//expectedSequenceNumber is updated in cwnd_addPkt
		cwnd_addPkt(cwndR, buf);
	}
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
			if(getACK(buf) == true /*&& not corrupt*/)
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

				if(getACKNum(buf) == cwnd_lastSeq(cwndW)){
					cwnd_setAck(cwndW, getACKNum(buf));
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
	uint32_t lastMss = cwnd_lastMss(cwndR);

	if(cwnd_numPendingAcks(cwndR) == 0)
	{
		printf("writeAckPacket: no packets unacknowledged\n");
		return false;
	}
	printf("writeAckPacket: lastMss=%d\n",lastMss);
	if(p_check(p_corr)) // corrupt packet
		memset(buf, 0, PACKET_SIZE);
	else
		memcpy(buf, (byte_t *)cwnd_getPktSeq(cwndR, getSeqNum(buf)),PACKET_SIZE);
	// printf("writeAckPacket: after memcpy\n");
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
	// cwnd_t *cwndR;
    cwnd_t *cwndW;

    // cwndR = cwnd_init(cwndR);
    cwndW = cwnd_init(cwndW);
    seed(); // seed random for packet loss and corruption


	//break up buf into packets
	pkts = bufToPackets(buf, nbytes);
	

	//write packets 
	while(cwnd_numPendingAcks(cwndW) > 0 || pkts[i] != 0)
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
   	
    // cwnd_free(cwndR);
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
	byte_t buf[PACKET_SIZE];

	cwnd_t *cwndR;
    // cwnd_t *cwndW;

    cwndR = cwnd_init(cwndR);
    // cwndW = cwnd_init(cwndW);


	if(msgBody != NULL)
	{
		free(msgBody);
		msgBody = NULL;
	}
	msgBody = malloc(MAX_BODY_SIZE);
	memset(msgBody, 0, MAX_BODY_SIZE);
	do{
		printf("readTCP: start do\n");
		uint8_t currLen = 0;
		
		// printf("readTCP: before readPacket\n");

		if(getLast(pkt) == false)
		{
			bytesrecv = readPacket(sockfd, sockaddr, socklen, cwndR);
		}
		//writeAck should work even if nothing was received
		printf("readTCP: after readPacket\n");
		cwnd_print(cwndR);
		printf("readTCP: no segfault yet\n");
		pkt = (byte_t *)cwnd_getPktSeq(cwnd_lastSeq(cwndR));
		if(pkt != NULL)
		{
			currLen += getSize(pkt);
			printf("readTCP: currLen=%d\n", currLen);
			printf("readTCP: currMsg=%s\n",getBody(pkt));
			//ack will mark the last packet read
			if(!p_check(p_loss))
				writeAckPacket(sockfd, sockaddr, socklen, cwndR, p_corr);
			msgBody = realloc(msgBody, msgLen + currLen);
			memcpy(&(msgBody[msgLen]), getBody(pkt), currLen);
			msgLen += currLen;
			//if(currLen < MAX_BODY_SIZE) break;//temp fix to waiting read
		}
		else{
			//sketchy code, quick fix
			printf("readTCP: wonky code\n");
			generatePacket(buf,0,0,0,0,0,"",0);
			pkt = buf;
		}
		printf("readTCP: bytesrecv=%d\n",bytesrecv);
	}while(getLast(pkt) == false && 
		   cwnd_numPendingAcks(cwndR) == 0);
	printf("readTCP: appended all of file, msgLen=%d\n", msgLen);
	// printf("readTCP: msg=%s\n",msgBody);
    cwnd_free(cwndR);
	// cwnd_free(cwndW);
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
