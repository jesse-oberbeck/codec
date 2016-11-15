#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <arpa/inet.h>

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
    uint32_t Dmac    : 32;
    uint16_t Dmac2   : 16;
    uint32_t Smac    : 32;
    uint16_t Smac2   : 16;
    uint32_t Etype   : 16;
};

struct __attribute__((__packed__)) Ipv4Header
{
    int IHL          : 4;
    int Version      : 4;
    int DSCP         : 6;
    int ECN          : 2;
    uint16_t TotalLen: 16;
    uint16_t Ident   : 16;
    int Flags        : 3;
    uint16_t FragOff : 13;
    uint8_t TTL      : 8;
    uint8_t Protocol : 8;
    uint16_t CheckSum: 16;
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
    uint32_t Type    : 4;
    uint32_t Version : 4;//////////
    uint32_t TotalLen: 24;
    uint32_t Sid     : 16;
    uint32_t Did     : 16;
    uint32_t Sequence: 32;
};

struct Message
{
    char *Message;
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
    //uint32_t Name   : 32;
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
    FILE *words = fopen("status.pcap", "rb");


    //int filesize = file_size(words);
    //char *contents = read_file(filesize, words);
    struct FileHeader *fh = calloc(sizeof(*fh),1); //file header
    struct PcapHeader *ph = calloc(sizeof(*ph),1); //packet header
    struct EthernetHeader *eh = calloc(sizeof(*eh),1); //ethernet header
    struct Ipv4Header *ih = calloc(sizeof(*ih),1); //ip header
    struct UdpHeader *uh = calloc(sizeof(*uh),1); //udp header
    struct ZergHeader *zh = calloc(sizeof(*zh),1); //zerg header

    fread(fh, sizeof(struct FileHeader), 1, words);
    fread(ph, sizeof(struct PcapHeader), 1, words);
    fread(eh, sizeof(struct EthernetHeader), 1, words);
    fread(ih, sizeof(struct Ipv4Header), 1, words);
    fread(uh, sizeof(struct UdpHeader), 1, words);
    fread(zh, sizeof(struct ZergHeader), 1, words);

    /*Probably unneeded MAC handling*/
    /*uint64_t macholder;
    uint64_t macholder2;
    macholder = htonl(eh->Dmac);
    macholder2 = htonl(eh->Dmac2);
    macholder2 = macholder2 >> 16;
    printf("Destination MAC: %3x", (int)(macholder));
    printf("%3x\n", (int)(macholder2));*/

    /*Printing Header Information*/
    printf("File Type: %x\n", (fh->FileType));
    printf("Major Version: %x\n", htonl(fh->MajorVer) >> 24);
    printf("Minor Version: %x\n", htonl(fh->MinorVer) >> 24);
    printf("Link Layer Type: %x\n\n", htonl(fh->LLT) >> 24);
    printf("Length of Data Captured: %x\n\n", htonl(ph->DataLen) >> 24);
    printf("Ethernet Type: %x\n", htonl(eh->Etype) >> 24);

    printf("IP Version: %x\n", ih->Version);
    printf("IHL: %x\n", ih->IHL);

    int zerg_type = htonl(zh->Type) >> 24;
    printf("Zerg Version: %x\n", htonl(zh->Version) >> 24);
    printf("Zerg Type: %x\n", zerg_type);
    printf("Sequence: %d\n", htonl(zh->Sequence));
    printf("Total Length: %d\n", htonl(zh->TotalLen) >> 8);

    if(zerg_type == 0)
    {
        int messageLen = (htonl(zh->TotalLen) >> 8) - 24;
        char *message = calloc(messageLen, 1);
        fread(message, htonl(zh->TotalLen) >> 8, 1, words);
        printf("%s\n", message);
    }
    
    if(zerg_type == 1)
    {
        struct Status *st = calloc(sizeof(*st),1);
        fread(st, sizeof(struct Status), 1, words);
        //char *name = ((htonl)(st->Name));
        //printf("Name: %s\n", name);
        //char *name = (char)(st->Name);
        int nameLen = (htonl(zh->TotalLen) >> 8) - 24;
        //printf("namelen: %d\n", nameLen);
        char *message = calloc(nameLen, 1);
        fread(message, nameLen, 1, words);
        printf("Name: %s\n", message);
        printf("HP: %d/%d\n", htonl(st->HP) >> 8, htonl(st->MaxHP) >> 8);

    }
    
}
