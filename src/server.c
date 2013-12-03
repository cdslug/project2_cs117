/*
 * server_spencer.c
 *
 *  Created on: Nov 26, 2013
 *      Author: spencer
 */


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
#include "tcp118.h"

#define MAX_FILE_SIZE 1000000
#define BUFSIZE 1024
#define TIMEOUT_MS 100


void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char * argv[]){
	int sockfd, portno; // sockfd:portno -> server socket
	int bytesrecv;  /* # bytes received */
	int cWnd;
    double p_loss, p_corr;
	struct sockaddr_in serv_addr; // server address
	struct sockaddr_in cli_addr; // client address
	struct timeval tv;
	socklen_t clilen = sizeof(cli_addr);

	char buf[BUFSIZE]; // receive buffer

	if(argc != 5){
    	error("ERROR, usage ./server portnumber CWnd P_l P_c");
	}

	portno = atoi(argv[1]);
	cWnd = atoi(argv[2]);
	p_loss = atof(argv[3]);
	p_corr = atof(argv[4]);

	/* create UDP socket */
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		error("ERROR, opening socket");
	}

	//Set timeout value for recvfrom
	tv.tv_sec = 0;
	tv.tv_usec = 1000*TIMEOUT_MS; // Set the timeout microsecond value using number of milliseconds
	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
		error("ERROR: set timeout option\n");

	/* bind UDP socket */
	memset((char *) &serv_addr, 0, sizeof(serv_addr)); //clear memory for socket
	//fill in address info
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portno);
	if((bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
		error("ERROR, binding socket");
	}

	while(1) {
		int fbytesread;
		char fileBuf[MAX_FILE_SIZE];
		byte_t * fileNamePtr[1] = {NULL};
		byte_t *fileName = NULL;

		printf("waiting on port %d\n", portno);
		readTCP(sockfd, (struct sockaddr *)&cli_addr, clilen, fileNamePtr,cWnd, p_loss, p_corr);
		fileName = *fileNamePtr;
		
	


		//read file into file buffer
		printf("server: Reading file...\n");
		printf("server: fileName=%s\n",fileName);
		fbytesread = readFile(fileName, fileBuf, MAX_FILE_SIZE);
		if(fbytesread > 0) {
 			printf("File Content: \n%s\n", fileBuf);
 			writeTCP(sockfd, (struct sockaddr *)&cli_addr, clilen, fileBuf, fbytesread,cWnd, p_loss, p_corr);
		}
		else
			printf("Error reading file!\n");
		free(fileName);
		
		
	}
	// never exits

}
