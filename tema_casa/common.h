#ifndef _COMMS_H_
#define _COMMS_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define TAG_SYNC    100
#define TAG_FREE	201
#define TAG_MAP		301
#define TAG_REDUCE	401
#define TAG_STOP	0x29A
#define DELIM_CHAR  '_'

#define MAX_FILE_NAME_LEN   20
#define MAX_PATH_LEN        500

FILE * openReadPipe(const char * lsCmd);

#endif
