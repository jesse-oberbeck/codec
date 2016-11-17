#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <math.h>
#include <inttypes.h>
#include <endian.h>
#include <unistd.h>

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
    uint64_t Longit;
    uint64_t Latit;
    int Altit  : 32;
    int Bearing: 32;
    int Speed  : 32;
    int Acc    : 32;
};

//Headers are 752 total bytes for a single set.

void process_file(FILE *words)
{
    struct FileHeader *fh = calloc(sizeof(*fh),1); //file header
    struct PcapHeader *ph = calloc(sizeof(*ph),1); //packet header
    struct EthernetHeader *eh = calloc(sizeof(*eh),1); //ethernet header
    struct Ipv4Header *ih = calloc(sizeof(*ih),1); //ip header
    struct UdpHeader *uh = calloc(sizeof(*uh),1); //udp header
    

    fread(fh, sizeof(struct FileHeader), 1, words);
    fread(ph, sizeof(struct PcapHeader), 1, words);
    fread(eh, sizeof(struct EthernetHeader), 1, words);
    fread(ih, sizeof(struct Ipv4Header), 1, words);
    fread(uh, sizeof(struct UdpHeader), 1, words);
    

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


    free(fh);
    free(ph);
    free(eh);
    free(ih);
    free(uh);

    return;
}

struct Container
{
    int zerg_type;
    int total_len;
};

void process_zerg_header(FILE *words, struct ZergHeader *zh, struct Container *c)
{
    fread(zh, sizeof(struct ZergHeader), 1, words);
    int zerg_type = htonl(zh->Type) >> 24;
    int total_len = htonl(zh->TotalLen) >> 8;
    printf("Zerg Version: %x\n", htonl(zh->Version) >> 24);
    printf("Zerg Type: %d\n", zerg_type);
    printf("Sequence: %d\n", htonl(zh->Sequence));
    printf("Total Length: %d\n", total_len);
    printf("Destination ID: %d\n", htonl(zh->Did) >> 16);
    printf("Source ID: %d\n", htonl(zh->Sid) >> 16);
    c->zerg_type = zerg_type;
    c->total_len = total_len;
    return;
}

/*Get end of file*/
int file_size(FILE *words)
{

    fseek(words, 0, SEEK_END);
    long end = ftell(words);
    fseek(words, 0, SEEK_SET);
    return(end);
}

int main(int argc, char *argv[])
{
    if( !(argc > 1) && !(access(argv[1], F_OK)) )
    {
        perror("Invalid file Name.");
        return(1);
    }

    char *file = argv[1];
    FILE *words = fopen(file, "rb");
    int eof = file_size(words);
    process_file(words);

    struct ZergHeader *zh = calloc(sizeof(*zh), 1); //zerg header
    struct Container *c = calloc(sizeof(*c), 1);
    
    while(ftell(words) != eof){
    process_zerg_header(words, zh, c);
    int zerg_type = c->zerg_type;
    int total_len = c->total_len;

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
        int nameLen = (htonl(zh->TotalLen) >> 8) - 24;
        //printf("namelen: %d\n", nameLen);
        char *message = calloc(nameLen, 1);
        fread(message, nameLen, 1, words);
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
        int mantissa = bin_speed & 0x7FFFFF;
        int exponent = (bin_speed >> 23) - 127;
        double speed =  pow(2, exponent) * mantissa;
        printf("sign: %d mantissa: %d exponent: %d\n", speed_sign, mantissa, exponent);
        */
        
        int bin_speed = htonl(st->Speed);
        //printf("bin_speed: %x\n", bin_speed);
        union{float f; uint32_t u;} converter; //stackoverflow.com/questions/15685181/how-to-get-the-sign-mantissa-and-exponent-of-a-floating-point-number
        converter.u = bin_speed;
        double speed = converter.f;
        printf("Max Speed: %fm/s\n", speed);
        converter.f = bin_speed;
        //int notspeed = converter.u;
        //printf("and back: %x\n", notspeed);
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
        /*//Didn't work, but I still like the idea.
        uint64_t intbuf = 0;
        int bin_latitude = htonl(gps->Latit);
        int bin_latitude2 = htonl(gps->Latit2);
        intbuf += bin_latitude2;
        intbuf = intbuf << 32;
        intbuf += bin_latitude;
        uint64_t combined_lat = intbuf;
        
        intbuf = 0;
        int bin_longitude = htonl(gps->Longit);
        int bin_longitude2 = htonl(gps->Longit2);
        intbuf += bin_longitude;
        intbuf = intbuf << 32;
        intbuf += bin_longitude2;
        uint64_t combined_long = intbuf;
        */

        //printf("bin_speed: %x\n", bin_speed);
        unsigned long long latitude = be64toh(gps->Latit);
        printf("sizeof: %zd\n", sizeof(latitude));
        printf("lat bytes: %llx\n", latitude);
        unsigned long long longitude = be64toh(gps->Longit);
        union{double f; uint64_t u;} converter; //stackoverflow.com/questions/15685181/how-to-get-the-sign-mantissa-and-exponent-of-a-floating-point-number
        converter.u = latitude;
        double latitude2 = converter.f;
        converter.u = longitude;
        double longitude2 = converter.f;
        if(latitude2 > 0)
        {
            printf("latitude: %.9f deg. N\n", latitude2);
        }
        else
        {
            printf("latitude: %.9f deg. S\n", fabs(latitude2));
        }
        
        if(longitude2 > 0)
        {
            printf("longitude: %.9f deg. E\n", longitude2);
        }
        else
        {
            printf("longitude: %.9f deg. W\n", fabs(longitude2));
        }
        
        //printf("longitude: %f\n", longitude2);
    }
    }
    free(zh);
    fclose(words);

}
