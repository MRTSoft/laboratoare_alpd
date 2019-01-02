#ifndef _WORKER_H_
#define _WORKER_H_

#include "comms.h"

extern char tmp_dir[500];

int runWorker(int master);
void map(const char * file);
void reduce(const char * file);

#endif
