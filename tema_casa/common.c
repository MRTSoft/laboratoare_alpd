#include "common.h"

FILE * openReadPipe(const char * lsCmd){
    FILE *fp;
    fp = popen(lsCmd, "r");
    return fp;
}