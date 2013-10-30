#include "chksum.h"
char* add_chksum(Frame* frame)
{
    frame->checksum = 0;
    char* buf;
    buf = convert_frame_to_char(frame);
    frame->checksum = chksum(
	(unsigned short *) buf, MAX_FRAME_SIZE / 2);
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

