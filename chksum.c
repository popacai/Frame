#include "chksum.h"
#include "crc.h"
#include "common.h"
//Although it is named as chksum, but we use the crc instead
char* add_chksum(Frame* frame)
{
    frame->nop = 0;
    frame->checksum = 0;
    char* buf;
    buf = convert_frame_to_char(frame);

    frame->checksum = chksum_all(buf);

    free(buf);
    buf = convert_frame_to_char(frame);
    return buf;
}
unsigned char chksum_all(char* buf)
{
    //return chksum((unsigned short *) buf, MAX_FRAME_SIZE / 2);
    unsigned char buffer[MAX_FRAME_SIZE];
    unsigned char crc;
    memcpy(buffer, buf, MAX_FRAME_SIZE);
    crcOp(buffer, MAX_FRAME_SIZE * 8);
    crc = buffer[0];

    return crc;
}
//This is the implementation of chksum
/*
char* add_chksum(Frame* frame)
{
    frame->checksum = 0;
    char* buf;
    buf = convert_frame_to_char(frame);
    frame->checksum = chksum(
	(unsigned short *) buf, MAX_FRAME_SIZE / 2);
    free(buf);
    buf = convert_frame_to_char(frame);
    return buf;
}
unsigned short chksum_all(char* buf)
{
    return chksum((unsigned short *) buf, MAX_FRAME_SIZE / 2);
}
unsigned short chksum(unsigned short *buf, int count)
{
    register unsigned long sum = 0;
    while (count--)
    {
	sum += *buf++;
	if (sum & 0xFFFF0000)
	{
	    sum &= 0xFFFF;
	    sum++;
	}
    }
    return ~(sum & 0xFFFF);
}

*/
