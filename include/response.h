#ifndef RESPONSE_H
#define RESPONSE_H

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "parse.h"

#define HTTP_VERSION 	0
#define STATUS 			1
#define STATUS_STR 		2
#define CONNECTION 		3
#define DATE 			4
#define SERVER 			5

#define CONTENT_TYPE  	6
#define CONTENT_LENGTH  7
#define LAST_MODIFIED 	8
#define BODY 			9

#define NUM_HEADER_ELEMENTS 10


#define PACKET_SIZE 1024 //files will be broken up into 1KB packets

typedef struct {
// //connection pointers
// 	char* HTTP_version;
// 	char* status;
// 	char* status_str;
// 	char* connection;
// 	char* date;
// 	char* server;
// //content pointers
// 	char* content_type;
// 	char* content_length;
// 	char* last_modified;
// 	char* body;
//pointers to all content
	char ** header_lines;
	char ** header_fields;
	size_t body_len;
	size_t msg_len;
	char * message;
} http_w;

typedef struct {
	char ** packets;
} file_packet; 

char** fileToPackets(char * file_s);

http_w * responseInit();

char * dateToStr();
char * numToStr(size_t num);
char * getContentType(const char * URI);
char * getFileDate(FILE * fp);
void getFileInfo(const http_r * request, http_w * response);

char * getStatusStr(int status);
http_w * generateResponseInfo(http_r * request);
http_w * generateResponseMessage(http_r * request);
void freeResponse(http_w * response);


#endif //RESPONSE_H