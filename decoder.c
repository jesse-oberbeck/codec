#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <math.h>
#include <inttypes.h>

struct __attribute__((__packed__)) FileHeader //stackoverflow.com/questions/4306186/structure-padding-and-packing
{
    int FileType: 32;
    int MajorVer: 16;
    int MinorVer: 16;
    int GMT     : 32;
    int Acc     : 32;
    int MaxLen  : 32;
    int LLT     : 32;
};

struct __attribute__((__packed__)) PcapHeader
{
    int Epoch   : 32;
    int EpochMil: 32;
    int DataLen : 32;
    int PackLen : 32;
};

struct __attribute__((__packed__)) EthernetHeader
{
    int Dmac    : 32;
    int Dmac2   : 16;
    int Smac    : 32;
    int Smac2   : 16;
    int Etype   : 16;
};

struct __attribute__((__packed__)) Ipv4Header
{
    int IHL          : 4;
    int Version      : 4;
    int DSCP         : 6;
    int ECN          : 2;
    int TotalLen: 16;
    int Ident   : 16;
    int Flags        : 3;
    int FragOff : 13;
    int TTL      : 8;
    int Protocol : 8;
    int CheckSum: 16;
    int SIP     : 32;
    int DIP     : 32;
};

struct __attribute__((__packed__)) UdpHeader
{
    int Sport   : 16;
    int Dport   : 16;
    int Len     : 16;
    int CheckSum: 16;
};

struct __attribute__((__packed__)) ZergHeader
{
    int Type    : 4;
    int Version : 4;//////////
    int TotalLen: 24;
    int Sid     : 16;
    int Did     : 16;
    int Sequence: 32;
};

struct Message
{
    char *Message;
    //Might not be needed. Entire payload is text.
    //"Not null terminated" so will have to cut off at packet len.
};

struct __attribute__((__packed__)) Status
{
    int HP     : 24;
    int Armor  : 8;
    int MaxHP  : 24;
    int Type   : 8;
    int Speed  : 32;
    //uint32_t Name   : 32;
};

struct __attribute__((__packed__)) Command
{
    int Command: 16;
    int Param1 : 16;
    int Param2 : 32;
};

struct __attribute__((__packed__)) GPS
{
    int Longit : 32;
    int Longit2: 32;
    int Latit  : 32;
    int Latit2 : 32;
    int Altit  : 32;
    int Bearing: 32;
    int Speed  : 32;
    int Acc    : 32;
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
    FILE *words = fopen("gps.pcap", "rb");


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

    /*  Printing Header Information  */
    //printf("File Type: %x\n", (fh->FileType));
    //printf("Major Version: %x\n", htonl(fh->MajorVer) >> 24);
    //printf("Minor Version: %x\n", htonl(fh->MinorVer) >> 24);
    //printf("Link Layer Type: %x\n\n", htonl(fh->LLT) >> 24);
    printf("Length of Data Captured: %d\n\n", htonl(ph->DataLen) >> 24);
    //printf("Ethernet Type: %x\n", htonl(eh->Etype) >> 24);

    //printf("IP Version: %x\n", ih->Version);
    //printf("IHL: %x\n", ih->IHL);

    int zerg_type = htonl(zh->Type) >> 24;
    int total_len = htonl(zh->TotalLen) >> 8;
    printf("Zerg Version: %x\n", htonl(zh->Version) >> 24);
    printf("Zerg Type: %d\n", zerg_type);
    printf("Sequence: %d\n", htonl(zh->Sequence));
    printf("Total Length: %d\n", total_len);
    printf("Destination ID: %d\n", htonl(zh->Did) >> 16);
    printf("Source ID: %d\n", htonl(zh->Sid) >> 16);

    if(zerg_type == 0)
    {
        char *message = calloc(total_len,1);
        fread(message, htonl(zh->TotalLen) >> 8, 1, words);
        printf("%s\n", message);
        free(message);
    }
    
    if(zerg_type == 1)
    {
        struct Status *st = calloc(sizeof(*st),1);
        fread(st, sizeof(struct Status), 1, words);
        //char *name = ((htonl)(st->Name));
        //printf("Name: %s\n", name);
        //char *name = (char)(st->Name);
        //int nameLen =5;
        char *message = calloc(total_len,1);
        fread(message, htonl(zh->TotalLen) >> 8, 1, words);
        printf("Name: %s\n", message);
        printf("HP: %d/%d\n", htonl(st->HP) >> 8, htonl(st->MaxHP) >> 8);
        printf("Type: %d\n", htonl(st->Type) >> 24);
        printf("Armor: %d\n", htonl(st->Armor) >> 8);
        
        /* I'm abandoning the manual method here. Seemed doable until the
        Mantissa portion. Also read an easier method, which will be used
        instead, and is the suggested method according to one comment.
        "Aliasing through pointer conversion is not supported by the C
        standard, and may be troublesome in some compilers."*/
        /*
        int bin_speed = htonl(st->Speed);
        int speed_sign = bin_speed >> 31;
        int that_weird_mantissa_thing = bin_speed & 0x7FFFFF;
        int exponent = (bin_speed >> 23) - 127;
        double speed =  pow(2, exponent) * that_weird_mantissa_thing;
        printf("sign: %d mantissa: %d exponent: %d\n", speed_sign, that_weird_mantissa_thing, exponent);
        */
        
        int bin_speed = htonl(st->Speed);
        printf("bin_speed: %x\n", bin_speed);
        union{float f; uint32_t u;} converter; //stackoverflow.com/questions/15685181/how-to-get-the-sign-mantissa-and-exponent-of-a-floating-point-number
        converter.u = bin_speed;
        double speed = converter.f;
        printf("Max Speed: %fm/s\n", speed);
        converter.f = bin_speed;
        int notspeed = converter.u;
        printf("and back: %x\n", notspeed);
        free(st);
        free(message);
        

    }
    
        if(zerg_type == 2)
    {
        struct Command *cm = calloc(sizeof(*cm),1);
        fread(cm, sizeof(struct Command), 1, words);
        int command = htonl(cm->Command);
        switch(command)
        {
            case(0):
                printf("GET_STATUS\n");
                break;
            case(1):
                printf("GOTO\n");
                break;
            case(2):
                printf("GET_GPS\n");
                break;
            case(3):
                printf("RESERVED\n");
                break;
            case(4):
                printf("RETURN\n");
                break;
            case(5):
                printf("SET_GROUP\n");
                break;
            case(6):
                printf("STOP\n");
                break;
            case(7):
                printf("REPEAT\n");
                break;
        }
        printf("P1: %d\n", htonl(cm->Param1));
        if(command % 2 == 0)
        {
            cm = realloc(cm, sizeof(cm) - 6);
        }
        else
        {
            printf("P2: %d\n", htonl(cm->Param2));
        }
        
        free(cm);
    }
    
        if(zerg_type == 3)
    {
        struct GPS *gps = calloc(sizeof(*gps),1);
        fread(gps, sizeof(struct GPS), 1, words);
        //int command = htonl(cm->Command);
        printf("Got GPS Packet.\n");
    }
    
    //Free all the things!
    fclose(words);
    free(fh);
    free(ph);
    free(eh);
    free(ih);
    free(uh);
    free(zh);
}
