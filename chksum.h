#include <stdio.h>
#include "common.h"
#include "util.h"

unsigned char chksum_all(char* buf);
//unsigned short chksum(unsigned short *buf, int count);
char* add_chksum(Frame* frame);
