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
#include "tcpglobal.h"
#include "cwnd118.h"
#include "packet118.h"
#include "probability.h"


int writePackets(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndW, double p_loss, double p_corr);
int readPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndR);


//to be used after sending a packet
bool readAckPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndW);
//to be used after reading a packet
bool writeAckPacket(int sockfd, struct sockaddr *sockaddr, socklen_t socklen, cwnd_t *cwndR, double p_loss, double p_corr);

//int acceptTCP(int sockFD, struct sockaddr *sockaddr, socklen_t socklen);
//int connectTCP(int sockFD, struct sockaddr *sockaddr, socklen_t socklen);

int writeTCP(int sockFD, struct sockaddr *socaddr, socklen_t socklen, byte_t * buf, size_t nbytes, double p_loss, double p_corr);
int readTCP(int sockFD, struct sockaddr *socaddr, socklen_t socklen, byte_t ** msgBody, double p_loss, double p_corr);



#endif //TCP118_H