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
#define TIMEOUT_MS 100

int main(int argc, char *argv[])
{
    int sockfd; //Socket descriptor
    int portno, bytesrecv;
    double p_loss, p_corr;
    char * filename; // name of file to be requested
    byte_t * filebodyPtr[1] = {NULL};
    byte_t * filebody;
    int nbytes = 0;
    char * hostname; // host name of server
    char buf[BUFLEN];
    struct sockaddr_in serv_addr; // server address
    struct timeval tv;
    socklen_t servlen = sizeof(serv_addr);
    struct hostent *server; //contains tons of information, including the server's IP address


    if (argc != 6) {
    	error("ERROR, usage ./client server_hostname server_portnumber filename P_l P_c");
    }
    hostname = argv[1];
    portno = atoi(argv[2]);
    filename = argv[3];
    p_loss = atof(argv[4]);
    p_corr = atof(argv[5]);

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

    //Set timeout value for recvfrom
    tv.tv_sec = 0;
    tv.tv_usec = 1000*TIMEOUT_MS; // Set the timeout microsecond value using number of milliseconds
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        error("ERROR: set timeout option\n");


    /* Request file from the server */
    memset(buf,0, BUFLEN);
    bcopy(filename, buf, strlen(filename));
    printf("Requesting file: %s\n", filename);
    writeTCP(sockfd, (struct sockaddr *)&serv_addr, servlen, filename, strlen(filename)+1,4096, p_loss, p_corr);
	
   //reading from server
    
    filebody = NULL;
    nbytes = readTCP(sockfd, (struct sockaddr *)&serv_addr, servlen, filebodyPtr,4096, p_loss, p_corr); 
    


    filebody=*filebodyPtr;
    if(filebody != NULL)
    {
        char * rcv_filename = malloc(sizeof(char)*(strlen(filename)+4+1));
        strcpy(rcv_filename, "rcv_");
        strcpy(&(rcv_filename[4]), filename);
        writeFile(rcv_filename, filebody, nbytes);
        free(rcv_filename);
    }
    printf("client: start\n\tbytes read=%d\nreadTCP=\n%s\nclient: end\n",nbytes, filebody);
    free(filebody);

    close(sockfd); //close socket
    return 0;
}
