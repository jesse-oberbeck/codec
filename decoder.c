#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>

struct __attribute__((__packed__)) FileHeader //stackoverflow.com/questions/4306186/structure-padding-and-packing
{
    long unsigned int FileType: 32;
    long unsigned int MajorVer: 16;
    long unsigned int MinorVer: 16;
    long unsigned int GMT     : 32;
    long unsigned int Acc     : 32;
    long unsigned int MaxLen  : 32;
    long unsigned int LLT     : 32;
};

struct __attribute__((__packed__)) PcapHeader
{
    long unsigned int Epoch   : 32;
    long unsigned int EpochMil: 32;
    long unsigned int DataLen : 32;
    long unsigned int PackLen : 32;
};

struct __attribute__((__packed__)) EthernetHeader
{
    long unsigned int Dmac    : 24;
    long unsigned int Dmac2   : 24;
    long unsigned int Smac    : 24;
    long unsigned int Smac2   : 24;
    long unsigned int Etype   : 16;
};

struct __attribute__((__packed__)) Ipv4Header
{
    long unsigned int Version : 4;
    long unsigned int IHL     : 4;
    long unsigned int DSCP    : 6;
    long unsigned int ECN     : 2;
    long unsigned int TotalLen: 16;
    long unsigned int Ident   : 16;
    long unsigned int Flags   : 3;
    long unsigned int FragOff : 13;
    long unsigned int TTL     : 8;
    long unsigned int Protocol: 8;
    long unsigned int CheckSum: 16;
    long unsigned int SIP     : 32;
    long unsigned int DIP     : 32;
};

struct __attribute__((__packed__)) UdpHeader
{
    long unsigned int Sport   : 16;
    long unsigned int Dport   : 16;
    long unsigned int Len     : 16;
    long unsigned int CheckSum: 16;
};

struct __attribute__((__packed__)) ZergHeader
{
    long unsigned int Version : 4;
    long unsigned int Type    : 4;//////////
    long unsigned int TotalLen: 24;
    long unsigned int Sid     : 16;
    long unsigned int Did     : 16;
    long unsigned int Sequence: 32;
};

struct Message
{
    //Might not be needed. Entire payload is text.
    //"Not null terminated" so will have to cut off at packet len.
};

struct __attribute__((__packed__)) Status
{
    long unsigned int HP     : 24;
    long unsigned int Armor  : 8;
    long unsigned int MaxHP  : 24;
    long unsigned int Type   : 8;
    long unsigned int Speed  : 32;
    long unsigned int Name   : 32;
};

struct __attribute__((__packed__)) Command
{
    long unsigned int Command: 16;
    long unsigned int Param1 : 16;
    long unsigned int Param2 : 32;
};

struct __attribute__((__packed__)) GPS
{
    long unsigned int Longit : 64;
    long unsigned int Latit  : 64;
    long unsigned int Altit  : 32;
    long unsigned int Bearing: 32;
    long unsigned int Speed  : 32;
    long unsigned int Acc    : 32;
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
    macholder = eh->Dmac;
    macholder2 = eh->Dmac2;
    printf("%x\n", htonl(zh->Sequence));
    printf("%x\n", htonl(zh->TotalLen));
    printf("%x", (int)(htonl)(macholder));
    printf("%x\n", (int)(htonl)(macholder2));
    //printf("%s\n", testpayload);
}