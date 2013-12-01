#include "cwnd118.h"

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
	printf("cwnd_getAck: seqNum=%d\n",seqNum%cwnd->size);
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
		if(cwnd->last_seq == cwnd->next_seq)
		{
			//TODO
			//start_timer
		}
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

byte_t * cwnd_getPktMss(cwnd_t *cwnd, uint32_t mss)
{
	//if the packet was acknowledged, do not return it
	if(cwnd_getAck(cwnd, mss*PACKET_SIZE) == 1)
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
	printf("cwnd_getPktSeq: seq=%d, ack=%d\n",seq,cwnd_getAck(cwnd, seq)%cwnd->size);
	if(cwnd_getAck(cwnd, seq) == 1)
	{
		return NULL;
	}
	else
	{

		return cwnd->packets[seq/PACKET_SIZE];
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
