#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <endian.h>
#include <arpa/inet.h>
#include "structures.h"

int get_value(char *string)
{
    int len = strlen(string);
    printf("string: %c len: %d\n", string[1], len);
    for(int i = 0; i < len; ++i)
    {
        if(isalpha(string[i]) != 0)
        {
            string[i] = ' ';
        }
    }
    int value = strtol(string, NULL, 10);
    return(value);
}

/*Get size of file*/
int file_size2(FILE *words)
{

    long start = ftell(words);
    fseek(words, 0, SEEK_END);
    long end = ftell(words);
    int filesize = (end - start) + 1;
    //printf("\nfilesize: %i\n", filesize);
    fseek(words, 0, SEEK_SET);
    return(filesize);
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
int
main(int argc,char *argv[])
{
//Check for file name provided as arg.
    if (argc < 2)
    {
        fprintf(stderr, "Please provide a file name.\n");
        return (1);
    }
    else if (access(argv[1], F_OK) == -1)
    {
        fprintf(stderr, "Invalid file name.\n");
        return(1);
    }

    char *file = argv[1];
    FILE *words = fopen(file, "rb");
    //int end_pos = file_size(words);
    char **lines = malloc(file_size2(words));




    //char *testheader = "\xD4\xC3\xB2\xA1";
    //FILE *words = fopen("testing.pcap", "wb+");
    //fwrite(testheader, strlen(testheader), 1, words);
    
    //char test[] = "check 123";
    //int val = get_value(test);
    //printf("val: %d\n", val);

    struct FileHeader *fh = calloc(sizeof(*fh), 1);
    struct PcapHeader *ph = calloc(sizeof(*ph), 1); //pcap header
    struct EthernetHeader *eh = calloc(sizeof(*eh), 1); //ethernet header
    struct Ipv4Header *ih = calloc(sizeof(*ih), 1); //ip header
    struct UdpHeader *uh = calloc(sizeof(*uh), 1);  //udp header
    struct ZergHeader *zh = calloc(sizeof(*zh), 1);
    
    (*fh).FileType = htonl((unsigned int)3569595041);
    (*fh).MajorVer = 2;
    (*fh).MinorVer = 4;
    (*fh).LLT = 1;

    (*ph).PackLen = htonl(108)>>24;
    (*ph).DataLen = htonl(108)>>24;

    (*eh).Etype = 2048;

    (*ih).Version = '\x4';
    (*ih).IHL = '\x5';
    (*ih).TotalLen = htonl(60)>>16;//Lenght of packet. 48 + payload
    (*ih).Protocol = '\x11';

    //uh.Dport
    //uh.Length

    (*zh).Version = '\x1';
    (*zh).Type = 0;
    (*zh).TotalLen = htonl(24)>>8;
    (*zh).Sid = htonl(1337)>>16;
    (*zh).Did = htonl(1234)>>16;
    
    //PAYLOAD
    
    (*ph).PackLen = 94;
    FILE *packet = fopen(argv[2], "wb+");
    fwrite(fh, sizeof(*fh), 1, packet);
    fwrite(ph, sizeof(*ph), 1, packet);
    fwrite(eh, sizeof(*eh), 1, packet);
    fwrite(ih, sizeof(*ih), 1, packet);
    fwrite(uh, sizeof(*uh), 1, packet);
    fwrite(zh, sizeof(*zh), 1, packet);
    fwrite("Hello World!", 12, 1, packet);
}
