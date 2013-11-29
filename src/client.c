/*
 * client_spencer.c
 *
 *  Created on: Nov 26, 2013
 *      Author: spencer
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>      // define structures like hostent
#include <stdlib.h>
#include <strings.h>
#include "tcp118.h"

void error(char *msg)
{
    perror(msg);
    exit(1);
}

#define BUFLEN 1024

int main(int argc, char *argv[])
{
    int sockfd; //Socket descriptor
    int portno, bytesrecv;
    char * filename; // name of file to be requested
    char * hostname; // host name of server
    char buf[BUFLEN];
    struct sockaddr_in serv_addr; // server address
    socklen_t servlen = sizeof(serv_addr);
    struct hostent *server; //contains tons of information, including the server's IP address


    if (argc != 4) {
    	error("ERROR, usage ./client server_hostname server_portnumber filename");
    }
    hostname = argv[1];
    portno = atoi(argv[2]);
    filename = argv[3];

    /* create UDP socket */
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) //create a new socket
        error("ERROR opening socket");

    server = gethostbyname(hostname); //takes a string like "www.yahoo.com", and returns a struct hostent which contains information, as IP address, address type, the length of the addresses...
    if (server == NULL) {
    	error("ERROR, host assignment");
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //initialize server's address
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Request file from the server */
    memset(buf,0, BUFLEN);
    bcopy(filename, buf, strlen(filename));
    printf("Requesting file: %s\n", filename);
	if (sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serv_addr, servlen)<0) {
		error("sendto");
	}
	//bool last = false;
	//while(!last){
		int pktcnt = 0;
		bytesrecv = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *)&serv_addr, &servlen);
		if(bytesrecv > 0){
			buf[bytesrecv] = 0;
			printf("received message: %s\n", buf);
			// send ACK
			sprintf(buf, "ack %d", pktcnt++);
			if (sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serv_addr, servlen)==-1) {
				error("sendto");
			}
		}
	//}
    close(sockfd); //close socket
    return 0;
}
