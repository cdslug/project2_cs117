/*
 * parse.h
 *
 *  Created on: Oct 28, 2013
 *      Author: spencer
 */

#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


typedef struct {
	char* method;
	char* URI;
	char* HTTP_version;
} http_r;


http_r* parseRequest(char* requestMessage);
void freeRequest(http_r* request);
void printRequest(http_r* request);

#endif //PARSE_H

