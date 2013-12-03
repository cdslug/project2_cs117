#include "cwnd118.h"

cwnd_t* cwnd_init(cwnd_t *cwnd, uint32_t cwnd_size)
{
	int i = 0;

	cwnd = malloc(sizeof(cwnd_t));

	cwnd->c_wnd = cwnd_size;
	cwnd->size = cwnd_size * 2;
	cwnd->start = 0;
	cwnd->last_seq = 0;
	cwnd->next_seq = 0;
	cwnd->acks = malloc(sizeof(int)*(cwnd_size*2/PACKET_SIZE));
	for(i = 0; i < (cwnd_size*2/PACKET_SIZE); i++)
	{
		cwnd->acks[i] = -1;
	}
	cwnd->packets = (byte_t **)malloc(sizeof(byte_t*)*(cwnd_size * 2/PACKET_SIZE));

	for(i = 0; i < (cwnd_size * 2)/PACKET_SIZE; i++)
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

void cwnd_start(cwnd_t *cwnd)
{
	cwnd->start = 1;
}
bool cwnd_getStarted(cwnd_t *cwnd)
{
	return cwnd->start;
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

uint32_t cwnd_maxMss(cwnd_t *cwnd)
{
	return cwnd->size/PACKET_SIZE;
}

uint32_t cwnd_maxSeq(cwnd_t *cwnd)
{
	return cwnd->size;
}

uint32_t cwnd_lastSeq(cwnd_t *cwnd)
{
	return cwnd->last_seq;
}

uint32_t cwnd_nextSeq(cwnd_t *cwnd)
{
	return cwnd->next_seq;
}

uint32_t cwnd_lastMss(cwnd_t *cwnd)
{
	//mod is just in case
	return (cwnd->last_seq/PACKET_SIZE) % cwnd->size;
}

uint32_t cwnd_nextMss(cwnd_t *cwnd)
{
	// printf("cwnd_nextMss: nextSeq=%d\n",cwnd->next_seq);
	//mod is just in case
	uint32_t next = (cwnd->next_seq/PACKET_SIZE) % cwnd->size;
	// printf("cwnd_nextMss: next=%d\n",next);
	return next;
}

int cwnd_numPendingAcks(cwnd_t *cwnd)
{
	int i = 0, count = 0; 

	for(i = 0; i < cwnd->size/PACKET_SIZE; i++)
	{
		if(cwnd_getAck(cwnd, i*PACKET_SIZE) == 0)
		{
			// printf("cwnd_numPendingAcks: acks[%d]=%d\n",i,cwnd->acks[i]);
			count++;
		}
	}
	// uint32_t num = ((uint32_t)cwnd_nextMss(cwnd)-(uint32_t)cwnd_lastMss(cwnd)) % (cwnd->size/PACKET_SIZE);
	// printf("cwnd_numPendingAcks: nextMss=%d, lastMss=%d\n",cwnd_nextMss(cwnd), cwnd_lastMss(cwnd));
	// printf("cwnd_numPendingAcks: num=%d\n",count);
	return count;
}

int cwnd_numReceivedAcks(cwnd_t *cwnd)
{
	int i = 0, count = 0; 

	for(i = 0; i < cwnd_maxMss(cwnd); i++)
	{
		if(cwnd_getAck(cwnd, i*PACKET_SIZE) == 1)
		{
			// printf("cwnd_numPendingAcks: acks[%d]=%d\n",i,cwnd->acks[i]);
			count++;
		}
	}
	// uint32_t num = ((uint32_t)cwnd_nextMss(cwnd)-(uint32_t)cwnd_lastMss(cwnd)) % (cwnd->size/PACKET_SIZE);
	// printf("cwnd_numPendingAcks: nextMss=%d, lastMss=%d\n",cwnd_nextMss(cwnd), cwnd_lastMss(cwnd));
	// printf("cwnd_numReceivedAcks: num=%d\n",count);
	return count;
}

void cwnd_setAck(cwnd_t *cwnd, uint32_t seqNum)
{
	int i = (seqNum%cwnd->size)/PACKET_SIZE;
	if(cwnd_checkIn(cwnd, seqNum))
	{
		cwnd->acks[i] = 1;
	}s
	else
	{
		printf("cwnd_setAck: seqNum out of frame");
	}
}

void cwnd_resetAck(cwnd_t *cwnd)
{
	int i = 0; 
	for(i = 0; i < cwnd->size/PACKET_SIZE; i++)
	{
		cwnd->acks[i] = -1;
	}
}

int cwnd_getAck(cwnd_t *cwnd, uint32_t seqNum)
{
	// printf("cwnd_getAck: seqNum=%d\n",seqNum%cwnd->size);
	// printf("cwnd_getAck: seq = seqNum\n");
	int i = (seqNum%cwnd->size)/PACKET_SIZE;
	// printf("cwnd_getACk: index`=%d\n",i);

	return cwnd->acks[i];
}	

void cwnd_setAllPrevAck(cwnd_t *cwnd, uint32_t seqNum)
{
	int i = cwnd_lastMss(cwnd);
	uint32_t nextSeq = cwnd->next_seq;
	if(seqNum < cwnd_lastSeq(cwnd))
	{
		seqNum+=cwnd->size;
	}
	if(nextSeq < cwnd_lastSeq(cwnd))
	{
		nextSeq+=cwnd->size;
	}
	if(seqNum < nextSeq)
	{
		// printf("cwnd_setAllPrevAck: seqNum=%d,lastSeq=%d,nextSeq=%d\n",seqNum, cwnd->last_seq, nextSeq);
		for(i =cwnd_lastMss(cwnd); i <= seqNum/PACKET_SIZE; i++)
		{
			// printf("cwnd_setAllPrevAck: i=%d\n", i);
			cwnd_setAck(cwnd, i*PACKET_SIZE);
		}
	}
	else
	{
		// printf("cwnd_setAllPrevAck: out of range\n");
	}
}
int cwnd_lastPendingAckMss(cwnd_t *cwnd)
{
	int i = 0;
	for(i = cwnd_lastMss(cwnd); 
		cwnd_getAck(cwnd, i*PACKET_SIZE) == 1 && i != cwnd_nextMss(cwnd);
		)
	{
		i = (i+1)%cwnd_maxMss(cwnd);
	}
	if(i != cwnd_nextMss(cwnd))
	{
		return i;
	}
	else 
	{
			return -1;
	}
}

bool cwnd_checkIn(cwnd_t *cwnd, uint32_t seqNum)
{
	// printf("cwnd_checkIn: seqNum=%d\n,last=%d\n,next=%d\n",seqNum, cwnd->last_seq, cwnd->next_seq);
	uint32_t tempNext = cwnd->next_seq;
	if(cwnd->next_seq < cwnd->last_seq)
	{
		tempNext += cwnd->size;
	}
	if(seqNum >= cwnd->last_seq && seqNum < tempNext)
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
	if((cwnd->next_seq - cwnd->last_seq) % cwnd->size >= cwnd->c_wnd){
		return false;
	}
	else{
		return true;
	}
}

bool cwnd_addPkt(cwnd_t *cwnd, byte_t *buf)
{
	//need to do checks first
	// printf("cwnd_addPkt: nextMss=%d\n", cwnd_nextMss(cwnd));
	//then update variables
	//if there are spots in the congestion window
	if(cwnd_checkAdd(cwnd))
	{

		// printf("cwnd_addPkt: adding\n");
		memcpy(cwnd->packets[cwnd_nextMss(cwnd)], buf, PACKET_SIZE);
		// printf("cwnd_addPkt: after memcpy, nextMss=%d\n",cwnd_nextMss(cwnd));
		// printPacket(buf);
		// printPacket(cwnd->packets[0]);

		cwnd->acks[cwnd_nextMss(cwnd)] = 0;
		if(cwnd->last_seq == cwnd->next_seq)
		{
			int i = 0;//dummy code
			//TODO
			//start_timer
		}
		cwnd->next_seq = (cwnd->next_seq+PACKET_SIZE) % cwnd->size;
		cwnd_start(cwnd);
		return true;
	}
	else{
		printf("cwnd_addPkt: window full!\n");
		return false;
	}
}

// int cwnd_lastPktIndex(cwnd_t *cwnd)
// {
// 	int i = cwnd_lastMss(cwnd);
// 	do{
// 		if(cwnd->acks[i] == 0)
// 		{
// 			printf("cwnd_lastPktIndex: index=%d\n",i);
// 			return i;
// 		}
// 		i = (i+1)%((C_WND*2)/PACKET_SIZE);
// 	}while(i != cwnd_lastMss(cwnd));
// 	return -1;
// }

byte_t * cwnd_getPktMss(cwnd_t *cwnd, uint32_t mss)
{
	//if the packet was acknowledged, do not return it
	if(cwnd_getAck(cwnd, mss*PACKET_SIZE) != 0)
	{
		return NULL;
	}
	else
	{
		return cwnd->packets[mss];
	}
}

byte_t * cwnd_getPktSeq(cwnd_t *cwnd, uint32_t seq)
{
	//if the packet was acknowledged, do not return it
	// printf("cwnd_getPktSeq: seq=%d, ack=%d\n",seq, cwnd_getAck(cwnd,0));
	if(cwnd_getAck(cwnd, seq) != 0)
	{
		// printf("cwnd_getPktSeq: null\n");
		return NULL;
	}
	else
	{
		// printf("cwnd_getPktSeq: returning packet\n");
		return cwnd->packets[seq/PACKET_SIZE];
	}
}

// void cwnd_markLastPktRead(cwnd_t *cwnd)
// {
// 	int i = cwnd_lastPktIndex(cwnd);
// 	printf("cwnd_markLastPktRead: index=%d\n",i);
// 	if(i >= 0 && i < cwnd_maxMss(cwnd))
// 	{
// 		cwnd->acks[i] = 1;
// 		cwnd->last_seq = (cwnd->last_seq+PACKET_SIZE)%cwnd->size;
// 	}
// 	else
// 	{
// 		printf("cwnd_markLastPktRead: ERROR index out of range\n");
// 	}
// 	printf("cwnd_markLastPktRead: state=%d\n", cwnd_getAck(cwnd,i*PACKET_SIZE));
// }

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

//set's the last_seq to the next_seq
//should be used once all ACKS for a window have been satisfied
void cwnd_shiftWnd(cwnd_t * cwnd)
{
	cwnd->last_seq = cwnd->next_seq;
	cwnd_resetAck(cwnd);
}
