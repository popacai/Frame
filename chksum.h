#include <stdio.h>
#include "common.h"
#include "util.h"

unsigned short chksum(unsigned short *buf, int count);
unsigned short calc_checksum(char* buf);
unsigned char calc_crc(char* buf);
unsigned char chksum_all(char* buf);
//unsigned short chksum(unsigned short *buf, int count);
char* add_chksum(Frame* frame);
