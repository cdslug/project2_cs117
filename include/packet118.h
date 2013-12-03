#ifndef PACKET118_H
#define PACKET118_H

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


uint16_t checksum(const uint8_t * addr, uint32_t count);

uint32_t getSeqNum(const byte_t * pkt);
void setSeqNum(byte_t * pkt, uint32_t seqNum);
uint32_t getACKNum(const byte_t * pkt);
void setACKNum(byte_t * pkt, uint32_t ACKNum);
bool getACK(const byte_t * pkt);
void setACK(byte_t * pkt, bool ACK);
bool getLast(const byte_t * pkt);
void setLast(byte_t * pkt, bool last);
bool getClose(const byte_t * pkt);
void setClose(byte_t * pkt, bool shake);
uint16_t getSize(const byte_t * pkt);
int setSize(byte_t * pkt, uint16_t size);
uint16_t getChecksum(const byte_t *pkt);
void setChecksum(byte_t * pkt, uint16_t checksum);
byte_t * getBody(const byte_t * pkt);
int setBody(byte_t * pkt, byte_t * buff, size_t count);

byte_t * generatePacket( byte_t * pkt,
					   uint32_t seq_num, 
					   uint32_t ack_num, 
					   bool ack, 
					   bool last,
					   bool close,
					   byte_t * buff,
					   size_t count);
void printPacket(byte_t * pkt);
void freePacket(byte_t * pkt);
void freePackets(byte_t **pkts);

byte_t** bufToPackets(byte_t * buf, uint32_t nbytes,uint32_t c_wnd);


#endif //PACKET118_H