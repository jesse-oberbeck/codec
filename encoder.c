#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <endian.h>
#include <arpa/inet.h>
#include "structures.h"

//PADDING: length_of_data - ip_len - 14
//94 bytes before payload



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
/*Read in file.*/
char * read_file(int filesize, FILE *words)
{
    char *contents = malloc(filesize);
    fread(contents, sizeof(char), filesize, words);
    fclose(words);
    return(contents);
}

/*Line count. Tokenizes file based on newline, 
increasing a counter on each word.
returns the value held in the counter.*/
int line_count(char *contents)
{
    char *word = strtok(contents, "\n");
    int wordcount = 0;
    while(word != NULL){
        wordcount++;
        word = strtok(NULL, "\n");
    }
    printf("file total wordcount: %d\n\n", wordcount);
    return(wordcount);
}

int packet_count(char *contents)
{
    char *word = strtok(contents, "~");
    int packetcount = 0;
    while(word != NULL){
        packetcount++;
        word = strtok(NULL, "~");
    }
    printf("file total packets: %d\n\n", packetcount - 1);
    return(packetcount - 1);
}

char ** initialize(int *packetcount, const char *filename)
{
    FILE *words = fopen(filename, "r");
    int filesize = file_size(words);
    char *contents = read_file(filesize, words);
    char *contents2 = malloc(filesize);
    strncpy(contents2, contents, strlen(contents));
    *packetcount = packet_count(contents);
    free(contents);
    char **content_array;
    content_array = malloc(*packetcount * (int)(sizeof(char*) + 1));    
    char *splitstring = strtok(contents2, "~");
    int i = 0;
    while(splitstring){
        content_array[i] = calloc(strlen(splitstring) + 1, 1);
        strncpy(content_array[i], splitstring, strlen(splitstring));
        i++;
        splitstring = strtok(NULL, "~");

    }
    free(contents2);
    return(content_array);
}

char ** setup(int *linecount, char *packet)
{
    char *contents = packet;
    /////
    char *contents2 = malloc(strlen(packet));
    strncpy(contents2, contents, strlen(contents));
    /////
    *linecount = line_count(contents);
    free(contents);
    char **content_array;
    content_array = malloc(*linecount * (int)(sizeof(char*) + 1));    
    char *splitstring = strtok(contents2, "\n");
    int i = 0;
    while(splitstring){
        content_array[i] = calloc(strlen(splitstring) + 1, 1);
        strncpy(content_array[i], splitstring, strlen(splitstring));
        i++;
        splitstring = strtok(NULL, "\n");

    }
    free(contents2);
    return(content_array);
}

/*Frees allocated space in array, then array itself.*/
void array_free(char **content_array, int wordcount)
{
    for(int i = 0; i <= wordcount; ++i){
        free(content_array[i]);
    }
    free(content_array);
}

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
    int packetcount = 0;
    int linecount = 0;
    char **packets = initialize(&packetcount, argv[1]);
    FILE *packet = fopen(argv[2], "wb+");
for(int i = 0; i < packetcount; ++i){
    linecount = 0;
    printf("\nCURRENT PACKET: %s\n",packets[i]);
    char **lines = setup(&linecount, packets[i]);
    int zerg_type = get_value(lines[0]);
    //int sequence = get_value(lines[1]) + 1;
    //int zerg_len = get_value(lines[2]);
    int did = get_value(lines[2]);
    int sid = get_value(lines[3]);

    struct FileHeader *fh = calloc(sizeof(*fh), 1);
    struct PcapHeader *ph = calloc(sizeof(*ph), 1); //pcap header
    struct EthernetHeader *eh = calloc(sizeof(*eh), 1); //ethernet header
    struct Ipv4Header *ih = calloc(sizeof(*ih), 1); //ip header
    struct UdpHeader *uh = calloc(sizeof(*uh), 1);  //udp header
    struct ZergHeader *zh = calloc(sizeof(*zh), 1);

    (*fh).FileType = htonl((unsigned int)3569595041); //"\xD4\xC3\xB2\xA1"
    (*fh).MajorVer = 2;
    (*fh).MinorVer = 4;
    (*fh).LLT = 1;

    (*ph).PackLen = htonl(108)>>24;
    (*ph).DataLen = htonl(108)>>24;

    (*eh).Etype = 2048;

    (*ih).Version = '\x4';
    (*ih).IHL = '\x5';
    (*ih).TotalLen = htonl(60)>>16;//Length of packet. 48 + payload
    (*ih).Protocol = '\x11';

    (*uh).Dport = 42766;
    //uh.Length

    (*zh).Version = '\x1';
    (*zh).Type = zerg_type;
    (*zh).TotalLen = htonl(24)>>8;
    (*zh).Sid = htonl(sid)>>16;
    (*zh).Did = htonl(did)>>16;

    (*ph).PackLen = 94;

    if(!packet)
    {
        fprintf(stderr, "Failed to open file!");
        return(1);
    }

    fwrite(fh, sizeof(*fh), 1, packet);


    if (zerg_type == 0)
    {
        int p_len = 94 + strlen(lines[4]);
        int total_len = htonl(p_len)>>24;
        (*ph).PackLen = total_len;
        (*ph).DataLen = total_len;
        (*ih).TotalLen = htonl(60)>>16;//Length of packet. 48 + payload
        fwrite(ph, sizeof(*ph), 1, packet);
        fwrite(eh, sizeof(*eh), 1, packet);
        fwrite(ih, sizeof(*ih), 1, packet);
        fwrite(uh, sizeof(*uh), 1, packet);
        fwrite(zh, sizeof(*zh), 1, packet);
        fwrite(lines[4], strlen(lines[4]), 1, packet);
    }
    else if (zerg_type == 1)
    {
        int p_len = 94 + sizeof(struct Status) + strlen(lines[4]);
        int total_len = htonl(p_len)>>24;
        (*ph).PackLen = total_len;
        (*ph).DataLen = total_len;
        (*ih).TotalLen = htonl(60)>>16;//Length of packet. 48 + payload
        //struct Status *zp = calloc(sizeof(*zp), 1);
        fwrite(ph, sizeof(*ph), 1, packet);
        fwrite(eh, sizeof(*eh), 1, packet);
        fwrite(ih, sizeof(*ih), 1, packet);
        fwrite(uh, sizeof(*uh), 1, packet);
        fwrite(zh, sizeof(*zh), 1, packet);
        zerg1_encode(lines, packet);
    }

    else if (zerg_type == 2)
    {
        int p_len = 94 + sizeof(struct Command);
        int total_len = htonl(p_len)>>24;
        (*ph).PackLen = total_len;
        (*ph).DataLen = total_len;
        (*ih).TotalLen = htonl(60)>>16;//Length of packet. 48 + payload
        //struct Command *zp = calloc(sizeof(*zp), 1);
        fwrite(ph, sizeof(*ph), 1, packet);
        fwrite(eh, sizeof(*eh), 1, packet);
        fwrite(ih, sizeof(*ih), 1, packet);
        fwrite(uh, sizeof(*uh), 1, packet);
        fwrite(zh, sizeof(*zh), 1, packet);
        zerg2_encode(lines, packet);
    }

    else if (zerg_type == 3)
    {
        int p_len = 94 + sizeof(struct GPS);
        int total_len = htonl(p_len)>>24;
        (*ph).PackLen = total_len;
        (*ph).DataLen = total_len;
        (*ih).TotalLen = htonl(60)>>16;//Length of packet. 48 + payload
        //struct GPS *zp = calloc(sizeof(*zp), 1);
        fwrite(ph, sizeof(*ph), 1, packet);
        fwrite(eh, sizeof(*eh), 1, packet);
        fwrite(ih, sizeof(*ih), 1, packet);
        fwrite(uh, sizeof(*uh), 1, packet);
        fwrite(zh, sizeof(*zh), 1, packet);
        zerg3_encode(lines, packet);
    }
    
    else
    {
        fprintf(stderr, "Invalid packet instructions. Closing.");
        free(fh);
        free(ph);
        free(eh);
        free(ih);
        free(uh);
        free(zh);
        fclose(packet);
        array_free(lines, linecount);
        return(1);
    }


    free(fh);
    free(ph);
    free(eh);
    free(ih);
    free(uh);
    free(zh);

    //array_free(lines, linecount);
}
    fclose(packet);
    return(0);
}
