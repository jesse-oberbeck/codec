

#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <math.h>
#include <inttypes.h>
#include <endian.h>
#include <unistd.h>
#include "structures.h"
int
process_file(
    FILE * words)
{

    struct PcapHeader *ph = calloc(sizeof(*ph), 1); //pcap header
    struct EthernetHeader *eh = calloc(sizeof(*eh), 1); //ethernet header
    struct Ipv4Header *ih = calloc(sizeof(*ih), 1); //ip header
    struct UdpHeader *uh = calloc(sizeof(*uh), 1);  //udp header



    int fail = fread(ph, sizeof(struct PcapHeader), 1, words);
    if(fail == 0)
    {
        return(-1);
    }
    fread(eh, sizeof(struct EthernetHeader), 1, words);
    fread(ih, sizeof(struct Ipv4Header), 1, words);
    fread(uh, sizeof(struct UdpHeader), 1, words);

    /*  Printing Header Information  */
    int length_of_data = htonl(ph->DataLen) >> 24;
    int ip_len = htonl(ih->TotalLen) >> 16;
    if(length_of_data == 0){
    printf("Length of Data Captured is %d.\nEmpty file.\n", length_of_data);
    }

    free(ph);
    free(eh);
    free(ih);
    free(uh);

    return(length_of_data - ip_len - 14);
}



void
process_zerg_header(
    FILE * words,
    struct ZergHeader *zh,
    struct Container *c)
{
    fread(zh, sizeof(struct ZergHeader), 1, words);
    int zerg_type = htonl(zh->Type) >> 24;
    int total_len = htonl(zh->TotalLen) >> 8;

    //printf("Zerg Version: %x\n", htonl(zh->Version) >> 24);
    printf("Message Type: %d\n", zerg_type);
    printf("Sequence: %d\n", htonl(zh->Sequence));
    printf("Zerg Packet Length: %d\n", total_len);
    printf("Destination ID: %d\n", htonl(zh->Did) >> 16);
    printf("Source ID: %d\n", htonl(zh->Sid) >> 16);
    c->zerg_type = zerg_type;
    c->total_len = total_len;
    return;
}

/*Get end of file */
int
file_size(
    FILE * words)
{

    fseek(words, 0, SEEK_END);
    long end = ftell(words);

    fseek(words, 0, SEEK_SET);
    return (end);
}

float
convert_32(
    uint32_t num)
{
    union
    {
        float f;
        uint32_t u;
    } converter;                //stackoverflow.com/questions/15685181/how-to-get-the-sign-mantissa-and-exponent-of-a-floating-point-number

    converter.u = num;
    float result = converter.f;

    return (result);
}

double
convert_64(
    uint64_t num)
{
    union
    {
        double f;
        uint64_t u;
    } converter;                //stackoverflow.com/questions/15685181/how-to-get-the-sign-mantissa-and-exponent-of-a-floating-point-number
    uint64_t zero = 0;

    converter.u = zero;
    converter.u = num;
    double result = converter.f;

    return (result);
}

