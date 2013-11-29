# ifndef TCP118_H
# define TCP118_H

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

#define PACKET_SIZE 1024
#define HEADER_SIZE 12
#define MAX_BODY_SIZE 1012
#define TIME_OUT 60
#define C_WND 4096

#define SEQ_NUM_OFFSET 	0
#define ACK_NUM_OFFSET 	32
#define ACK_OFFSET 		64
#define LAST_OFFSET		65
#define SHAKE_OFFSET	66
#define SIZE_OFFSET		70
#define CHECKSUM_OFFSET	80
#define BODY_OFFSET     96

typedef uint8_t byte_t;
// typedef struct {
// 	uint32_t seq_num;
// 	uint32_t ack_num;
// 	uint16_t bits_and_size;
// 	uint16_t checksum;
// 	char *	 str;
// } packet_t;

//when writing, ack is 1 if an ack was received
//when reading, ack is 1 if a packet body was read
typedef struct {
	uint32_t c_wnd;
	uint32_t size;
	uint32_t last_seq;
	uint32_t next_seq;
	uint8_t *acks;
	byte_t ** packets;
} cwnd_t;

cwnd_t *cwnd_init(cwnd_t *cwnd);
void cwnd_free(cwnd_t *cwnd);
uint32_t cwnd_nextMss(cwnd_t *cwnd);
uint32_t cwnd_lastMss(cwnd_t *cwnd);
uint32_t cwnd_numPendingAcks(cwnd_t *cwnd);
bool cwnd_getAck(cwnd_t *cwnd, uint32_t seqNum);
bool cwnd_checkIn(cwnd_t *cwnd, uint32_t seqNum);
bool cwnd_checkAdd(cwnd_t *cwnd);
bool cwnd_addPkt(cwnd_t *cwnd, byte_t *buf);
int cwnd_lastPktIndex(cwnd_t *cwnd);
byte_t * cwnd_getLastPkt(cwnd_t *cwnd);
void cwnd_markLastPktRead(cwnd_t *cwnd);

uint16_t checksum(const uint8_t * addr, uint32_t count);

uint32_t getSeqNum(const byte_t * pkt);
void setSeqNum(byte_t * pkt, uint32_t seqNum);
uint32_t getACKNum(const byte_t * pkt);
void setACKNum(byte_t * pkt, uint32_t ACKNum);
bool getACK(const byte_t * pkt);
void setACK(byte_t * pkt, bool ACK);
bool getLast(const byte_t * pkt);
void setLast(byte_t * pkt, bool last);
bool getShake(const byte_t * pkt);
void setShake(byte_t * pkt, bool shake);
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
					   bool shake,
					   byte_t * buff,
					   size_t count);
void printPacket(byte_t * pkt);
void freePacket(byte_t * pkt);
void freePackets(byte_t **pkts);

byte_t** bufToPackets(byte_t * buf, uint32_t nbytes);

int writePacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndW, double p_corr);
int readPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndR);


//to be used after sending a packet
bool readAckPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndW);
//to be used after reading a packet
bool writeAckPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndR, double p_corr);

//int acceptTCP(int sockFD, struct sockaddr *sockaddr, socklen_t socklen);
//int connectTCP(int sockFD, struct sockaddr *sockaddr, socklen_t socklen);

int writeTCP(int sockFD, struct sockaddr *socaddr, socklen_t socklen, byte_t * buf, size_t nbytes, double p_loss, double p_corr);
int readTCP(int sockFD, struct sockaddr *socaddr, socklen_t socklen, byte_t * msgBody, double p_loss, double p_corr);



#endif //TCP118_H