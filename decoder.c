#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>

struct __attribute__((__packed__)) FileHeader //stackoverflow.com/questions/4306186/structure-padding-and-packing
{
    uint32_t FileType: 32;
    uint32_t MajorVer: 16;
    uint32_t MinorVer: 16;
    uint32_t GMT     : 32;
    uint32_t Acc     : 32;
    uint32_t MaxLen  : 32;
    uint32_t LLT     : 32;
};

struct __attribute__((__packed__)) PcapHeader
{
    uint32_t Epoch   : 32;
    uint32_t EpochMil: 32;
    uint32_t DataLen : 32;
    uint32_t PackLen : 32;
};

struct __attribute__((__packed__)) EthernetHeader
{
    uint32_t Dmac    : 24;
    uint32_t Dmac2   : 24;
    uint32_t Smac    : 24;
    uint32_t Smac2   : 24;
    uint32_t Etype   : 16;
};

struct __attribute__((__packed__)) Ipv4Header
{
    uint32_t Version : 4;
    uint32_t IHL     : 4;
    uint32_t DSCP    : 6;
    uint32_t ECN     : 2;
    uint32_t TotalLen: 16;
    uint32_t Ident   : 16;
    uint32_t Flags   : 3;
    uint32_t FragOff : 13;
    uint32_t TTL     : 8;
    uint32_t Protocol: 8;
    uint32_t CheckSum: 16;
    uint32_t SIP     : 32;
    uint32_t DIP     : 32;
};

struct __attribute__((__packed__)) UdpHeader
{
    uint32_t Sport   : 16;
    uint32_t Dport   : 16;
    uint32_t Len     : 16;
    uint32_t CheckSum: 16;
};

struct __attribute__((__packed__)) ZergHeader
{
    uint32_t Version : 4;
    uint32_t Type    : 4;//////////
    uint32_t TotalLen: 24;
    uint32_t Sid     : 16;
    uint32_t Did     : 16;
    uint32_t Sequence: 32;
};

struct Message
{
    //Might not be needed. Entire payload is text.
    //"Not null terminated" so will have to cut off at packet len.
};

struct __attribute__((__packed__)) Status
{
    uint32_t HP     : 24;
    uint32_t Armor  : 8;
    uint32_t MaxHP  : 24;
    uint32_t Type   : 8;
    uint32_t Speed  : 32;
    uint32_t Name   : 32;
};

struct __attribute__((__packed__)) Command
{
    uint32_t Command: 16;
    uint32_t Param1 : 16;
    uint32_t Param2 : 32;
};

struct __attribute__((__packed__)) GPS
{
    uint64_t Longit : 64;
    uint64_t Latit  : 64;
    uint32_t Altit  : 32;
    uint32_t Bearing: 32;
    uint32_t Speed  : 32;
    uint32_t Acc    : 32;
};

/*Read in file.*/
char * read_file(int filesize, FILE *words)
{
    char *contents = malloc(filesize);
    fread(contents, sizeof(char), filesize, words);
    fclose(words);
    return(contents);
}

//Headers are 752 total bytes for a single set.

/*Get size of file*/
int file_size(FILE *words)
{

    long start = ftell(words);
    fseek(words, 0, SEEK_END);
    long end = ftell(words);
    int filesize = (end - start) + 1;
    printf("\nfilesize: %i\n", filesize);
    fseek(words, 0, SEEK_SET);
    return(filesize);
}

int main(void)
{
    uint64_t macholder;
    uint64_t macholder2;
    FILE *words = fopen("hello.pcap", "rb");
    //int filesize = file_size(words);
    //char *contents = read_file(filesize, words);
    struct FileHeader *fh = calloc(sizeof(*fh),1); //file header
    struct PcapHeader *ph = calloc(sizeof(*ph),1); //packet header
    struct EthernetHeader *eh = calloc(sizeof(*eh),1); //ethernet header
    struct Ipv4Header *ih = calloc(sizeof(*ih),1); //ip header
    struct UdpHeader *uh = calloc(sizeof(*uh),1); //udp header
    struct ZergHeader *zh = calloc(sizeof(*zh),1); //zerg header
    //char testpayload[sizeof((zh->TotalLen)) - 64] = {'\0'};
    char *testpayload;
    fread(fh, sizeof(struct FileHeader), 1, words);
    fread(ph, sizeof(struct PcapHeader), 1, words);
    fread(eh, sizeof(struct EthernetHeader), 1, words);
    fread(ih, sizeof(struct Ipv4Header), 1, words);
    fread(uh, sizeof(struct UdpHeader), 1, words);
    fread(zh, sizeof(struct ZergHeader), 1, words);
    //fread(testpayload, sizeof(char*), 1, words);
    puts("tp print");
    macholder = htonl(eh->Smac);
    macholder2 = htonl(eh->Smac2);
    printf("%x\n", htonl(zh->Sequence));
    printf("%x\n", htonl(zh->TotalLen));
    printf("%3x ", (int)(macholder));
    printf("%3x\n", (int)(macholder2));
    //printf("%s\n", testpayload);
}