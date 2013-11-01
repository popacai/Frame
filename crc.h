#include "common.h"
char calc_crc(unsigned char *data);
int shift(unsigned char *data, int size);
void crcOp(unsigned char * data, int size); // size = sizeof(data);
