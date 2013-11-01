#include <stdio.h>
#include "crc.h"
char calc_crc(unsigned char *data)
{
    unsigned char buf[MAX_FRAME_SIZE];
    memcpy(buf, data, MAX_FRAME_SIZE);

    //rightest buf[MAX_FRAME_SIZE - 1] is the crc;
    crcOp(buf, MAX_FRAME_SIZE * 8);
    return buf[0];
}

int shift(unsigned char *data, int size)
{
    int i;
    for (i = 0; i < size; i++) {
        char x = (data[i] & 128) >> 7;
        data[i] <<= 1;
        if (i > 0) {
            data[i-1] |= x;
        }
    }
    return 1;
}
void crcOp(unsigned char * data, int size) // size = sizeof(data) * 8
{
    unsigned short crc = 32932;
    int length = size / 8;
    
    while (size > 8) {
        while (!(data[0] & 128)) {
            shift(data, length);
            size--;
	    if (size < 0)
		break;
            if (size % 8 == 0) {
                length--;
            }
        }
        unsigned short *tmp = (unsigned short *)data;
        if (size > 8) {
            *tmp = *tmp ^crc;
        }
    }
    if (!size)
    {
	data[0] = 0;
    }
    if (size < 8) {
        data[0] >>= 8 - size;
    }
}
int test_crc()
{
    unsigned char s[64];
    unsigned char t[64];
    unsigned char tmp;
    int i;
    for (i = 0; i < 64; i++)
    {
	tmp = rand() % 256;
	s[i] = tmp;
	t[i] = tmp;
    }
    s[63] = 0;
//    s[64] = 0xff;
    crcOp(s, 64 * 8);

    t[63] = s[0];
//    t[64] = 0xff;

    crcOp(t, 64 * 8);
    tmp = t[0];
    if (tmp)
    {
        printf("%d\n",(int)tmp);
    }

    return 0;
}

/*
int main()
{
    while (1){
        test_crc();
    }
    return 0;
}
*/
