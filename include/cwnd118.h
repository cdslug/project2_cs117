# ifndef CWND118_H
# define CWND118_H

#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>	/* for the waitpid() system call */
#include <signal.h>	/* signal name macros, and the kill() prototype */
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>
#include "tcpglobal.h"

typedef struct {
	uint32_t c_wnd;
	uint32_t size;
	uint8_t start;
	uint32_t last_seq;
	uint32_t next_seq;
	int *acks;
	byte_t ** packets;
} cwnd_t;

//when writing, ack is 1 if an ack was received
//when reading, ack is 1 if a packet body was read
cwnd_t *cwnd_init(cwnd_t *cwnd, uint32_t cwnd_size); 
void cwnd_free(cwnd_t *cwnd);
void cwnd_start(cwnd_t *cwnd);
bool cwnd_getStarted(cwnd_t *cwnd);
uint32_t cwnd_maxMss(cwnd_t *cwnd);
uint32_t cwnd_maxSeq(cwnd_t *cwnd);
uint32_t cwnd_lastSeq(cwnd_t *cwnd);
uint32_t cwnd_nextSeq(cwnd_t *cwnd);
uint32_t cwnd_lastMss(cwnd_t *cwnd);
uint32_t cwnd_nextMss(cwnd_t *cwnd);
int cwnd_numPendingAcks(cwnd_t *cwnd);
void cwnd_setAck(cwnd_t *cwnd, uint32_t seqNum);
void cwnd_resetAck(cwnd_t *cwnd);
int cwnd_getAck(cwnd_t *cwnd, uint32_t seqNum);
void cwnd_setAllPrevAck(cwnd_t *cwnd, uint32_t seqNum);
int cwnd_lastPendingAckMss(cwnd_t *cwnd);
bool cwnd_checkIn(cwnd_t *cwnd, uint32_t seqNum);
bool cwnd_checkAdd(cwnd_t *cwnd);
bool cwnd_addPkt(cwnd_t *cwnd, byte_t *buf);
// int cwnd_lastPktIndex(cwnd_t *cwnd);
byte_t * cwnd_getPktMss(cwnd_t *cwnd, uint32_t mss);
byte_t * cwnd_getPktSeq(cwnd_t *cwnd, uint32_t seq);
// void cwnd_markLastPktRead(cwnd_t *cwnd);
void cwnd_shiftWnd(cwnd_t * cwnd);

void cwnd_print(cwnd_t *cwnd);

#endif //CWND118_H