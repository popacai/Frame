#include "chksum.h"
#include "crc.h"
#include "common.h"

//We use both checksum and crc here
//Although the function name is chksum.
char* add_chksum(Frame* frame)
{
    frame->crc = 0;
    frame->nop = 0;
    frame->checksum = 0;

    //calc the checksum;
    char* buf;
    buf = convert_frame_to_char(frame);

    frame->checksum  = calc_checksum(buf);
    free(buf);

    //calc the crc
    buf = convert_frame_to_char(frame);

    frame->crc = calc_crc(buf);
    free(buf);

    buf = convert_frame_to_char(frame);
    return buf;
}
unsigned char chksum_all(char* buf)
{
    //check for checksum
    short checksum;
    checksum = calc_checksum(buf);
    if (checksum)
    {
	return 1;
    }

    //check for crc
    unsigned char crc;
    crc = calc_crc(buf);
    if (crc)
    {
	return 1;
    }
    return 0;
}
unsigned char calc_crc(char* buf)
{
    unsigned char buffer[MAX_FRAME_SIZE];
    unsigned char crc;
    memcpy(buffer, buf, MAX_FRAME_SIZE);
    crcOp(buffer, MAX_FRAME_SIZE * 8);
    crc = buffer[0];
    return crc;
}

unsigned short calc_checksum(char* buf)
{
    //part length
    return chksum((unsigned short *) buf, (MAX_FRAME_SIZE - 2) / 2);
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


*/
