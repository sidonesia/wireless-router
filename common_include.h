#ifndef COMMON_INCLUDE_H
#define COMMON_INCLUDE_H

	#include <stdio.h>
	#include <time.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <string.h>
	#include <assert.h>
	#include "table.h"

#define TRUE  1
#define FALSE 0
#define ERR  -1

#define PORT_CLEAR 7000


typedef int BOOLEAN;


char* convertPortToIpFormat
		(
			int Network,
			int interface,
			char* buffer
		);

char* getTimeNow(char* timebuff);

#endif
