#ifndef TCPGLOBAL_H
#define TCPGLOBAL_H



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

#endif //TCPGLOBAL_H