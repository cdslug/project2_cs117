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

#define PACKET_SIZE 1024
#define HEADER_SIZE 12
#define MAX_BODY_SIZE 1012
#define TIME_OUT 60
#define C_WND 4096

#define SEQ_NUM_OFFSET 	0
#define ACK_NUM_OFFSET 	32
#define ACK_OFFSET 		64
#define LAST_OFFSET		65
#define SIZE_OFFSET		70
#define CHECKSUM_OFFSET	80

typedef struct {
	uint32_t seq_num;
	uint32_t ack_num;
	uint16_t bits_and_size;
	uint16_t checksum;
	char *	 str;
} packet_t;

typedef struct {
	uint32_t c_wnd;
	uint32_t size;
	uint32_t next_seq;
	uint32_t next_mss;
	uint8_t *acks;
	packet_t * packets;
} cwnd_t;

uint16_t checksum(const uint8_t * addr, uint32_t count);

uint32_t getSeqNum(const packet_t * pkt);
void setSeqNum(packet_t * pkt, uint32_t);
uint32_t getACKNum(const packet_t * pkt);
void setACKNum(packet_t * pkt, uint32_t);
uint8_t getACK(const packet_t * pkt);
void setACK(packet_t * pkt, uint8_t);
uint8_t getLast(const packet_t * pkt);
void setLast(packet_t * pkt, uint8_t);
char * getData(const packet_t * pkt);
void setData(packet_t * pkt, char * buff, size_t count);

packet_t * generatePacket(uint32_t seq_num, 
					   uint32_t ack_num, 
					   uint8_t ack, 
					   uint8_t last,
					   const char * buff,
					   size_t count);

writeTCP(int * file_p, const char * buf, size_t nbytes);
char** strToPackets(char * file_s);
#endif //TCP118_H