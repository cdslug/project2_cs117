#ifndef PACKET118_H
#define PACKET118_H


typedef uint8_t byte_t;
// typedef struct {
// 	uint32_t seq_num;
// 	uint32_t ack_num;
// 	uint16_t bits_and_size;
// 	uint16_t checksum;
// 	char *	 str;
// } packet_t;




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


#endif //PACKET118_H