#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <endian.h>
#include <arpa/inet.h>
#include "structures.h"


/*Read in file.*/
char * read_file(
    int filesize,
    FILE *words)
{
    char *contents = calloc(filesize + 1, 1);
    fread(contents, sizeof(char), filesize, words);
    fclose(words);
    return(contents);
}

/*Line count. Tokenizes file based on newline, 
increasing a counter on each word.
returns the value held in the counter.*/
int line_count(
    char *contents)
{
    char *word = strtok(contents, "\n");
    int wordcount = 0;
    while(word != NULL){
        wordcount++;
        word = strtok(NULL, "\n");
    }
    return(wordcount);
}

int packet_count(
    char *contents)
{
    char *word = strtok(contents, "~");
    int packetcount = 0;
    while(word != NULL){
        ++packetcount;
        word = strtok(NULL, "~");
    }
    return(packetcount - 1);
}

char ** initialize(
    int *packetcount,
    const char *filename)
{
    FILE *words = fopen(filename, "r");
    int filesize = file_size(words);
    char *contents = read_file(filesize, words);
    char *contents2 = calloc(filesize + 1, 1);
    strncpy(contents2, contents, strlen(contents));
    *packetcount = packet_count(contents);
    char **content_array;
    content_array = calloc((*packetcount + 1) * (int)(sizeof(char*)), 1);
    char *splitstring = strtok(contents2, "~");
    int i = 0;
    while(splitstring != NULL){
        content_array[i] = calloc(strlen(splitstring) + 1, 1);
        strncpy(content_array[i], splitstring, strlen(splitstring));
        i++;
        splitstring = strtok(NULL, "~");

    }
    free(content_array[*packetcount]);
    free(contents);
    free(contents2);
    return(content_array);
}

char ** setup(
    int *linecount,
    char *packet)
{
    char *contents = packet;
    char *contents2 = calloc(strlen(packet) + 1, 1);
    strncpy(contents2, contents, strlen(contents));
    *linecount = line_count(contents);
    char **content_array;
    content_array = calloc(*linecount * (int)(sizeof(char*) + 1), 1);    
    char *splitstring = strtok(contents2, "\n");
    int i = 0;
    while((splitstring) && strcmp(splitstring,"\n") != 0){
        content_array[i] = calloc(strlen(splitstring) + 1, 1);
        strncpy(content_array[i], splitstring, strlen(splitstring));
        i++;
        splitstring = strtok(NULL, "\n");

    }
    free(contents);
    free(contents2);
    return(content_array);
}

/*Frees allocated space in array, then array itself.*/
void array_free(
    char **content_array,
    int wordcount)
{
    for(int i = 0; i <= wordcount; ++i){
        free(content_array[i]);
    }
    free(content_array);
}

int
main(
    int argc,
    char *argv[])
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
    struct FileHeader *fh = calloc(sizeof(*fh), 1);
    (*fh).FileType = htonl((unsigned int)3569595041); //"\xD4\xC3\xB2\xA1"
    (*fh).MajorVer = 2;
    (*fh).MinorVer = 4;
    (*fh).LLT = 1;
    fwrite(fh, sizeof(*fh), 1, packet);
    free(fh);

    for(int i = 0; i < packetcount; ++i){
        linecount = 0;
        char **lines = setup(&linecount, packets[i]);
        int zerg_type = get_value(lines[0]);
        //int sequence = get_value(lines[1]) + 1;
        //int zerg_len = get_value(lines[2]);
        int did = get_value(lines[2]);
        int sid = get_value(lines[3]);


        struct PcapHeader *ph = calloc(sizeof(*ph), 1); //pcap header
        struct EthernetHeader *eh = calloc(sizeof(*eh), 1); //ethernet header
        struct Ipv4Header *ih = calloc(sizeof(*ih), 1); //ip header
        struct UdpHeader *uh = calloc(sizeof(*uh), 1);  //udp header
        struct ZergHeader *zh = calloc(sizeof(*zh), 1);

        (*eh).Etype = 2048;

        (*ih).Version = '\x4';
        (*ih).IHL = '\x5';
        (*ih).Protocol = '\x11';

        (*uh).Dport = 42766;
        //uh.Length

        (*zh).Version = '\x1';
        (*zh).Type = zerg_type;
        (*zh).Sid = htonl(sid)>>16;
        (*zh).Did = htonl(did)>>16;

        (*ph).PackLen = 94;

        if(!packet)
        {
            fprintf(stderr, "Failed to open file!");
            free(ph);
            free(eh);
            free(ih);
            free(uh);
            free(zh);
            return(1);
        }



        --linecount;
        if (zerg_type == 0)
        {
            int zerglen = 12 + strlen(lines[4]);
            int p_len = 42 + zerglen;
            int total_len = htonl(p_len)>>24;
            int ip_len = 28 + zerglen;
            (*uh).Len = htonl(8 + zerglen);
            (*ph).PackLen = total_len;
            (*ph).DataLen = total_len;
            (*ih).TotalLen = htonl(ip_len)>>16;//Length of packet. 48 + payload
            (*zh).TotalLen = htonl(zerglen)>>8;
            fwrite(ph, sizeof(*ph), 1, packet);
            fwrite(eh, sizeof(*eh), 1, packet);
            fwrite(ih, sizeof(*ih), 1, packet);
            fwrite(uh, sizeof(*uh), 1, packet);
            fwrite(zh, sizeof(*zh), 1, packet);
            fwrite(lines[4], strlen(lines[4]), 1, packet);
        }
        else if (zerg_type == 1)
        {
            int zerglen = (strlen(lines[4]) - 6);
            int p_len = 54 + zerglen;
            int total_len = htonl(p_len)>>24;
            int ip_len = 40 + zerglen;
            (*uh).Len = htonl(8 + zerglen);
            (*ph).PackLen = total_len;
            (*ph).DataLen = total_len;
            (*ih).TotalLen = htonl(ip_len)>>16;//Length of packet. 48 + payload
            
            (*zh).TotalLen = htonl(zerglen)>>8;
            fwrite(ph, sizeof(*ph), 1, packet);
            fwrite(eh, sizeof(*eh), 1, packet);
            fwrite(ih, sizeof(*ih), 1, packet);
            fwrite(uh, sizeof(*uh), 1, packet);
            fwrite(zh, sizeof(*zh), 1, packet);
            zerg1_encode(lines, packet);
        }

        else if (zerg_type == 2)
        {
            int command_num = zerg2_encode(lines);
            int p_len = 0;
            int total_len = 0;
            int ip_len = 0;
            int udpLength = 0;
            if(command_num % 2 == 0)
            {
                p_len = 54 + 2;
                udpLength = 20 + 2;
                total_len = htonl(p_len)>>24;
                ip_len = 40 + 2;
            }
            else
            {
                p_len = 54 + 8;
                udpLength = 20 + 8;
                total_len = htonl(p_len)>>24;
                ip_len = 40 + 8;
            }

            (*zh).TotalLen = htonl(udpLength - 8)>>8;
            (*ph).PackLen = total_len;
            (*ph).DataLen = total_len;
            (*uh).Len = htonl(udpLength);
            (*ih).TotalLen = htonl(ip_len)>>16;//Length of packet. 48 + payload
            //struct Command *zp = calloc(sizeof(*zp), 1);
            fwrite(ph, sizeof(*ph), 1, packet);
            fwrite(eh, sizeof(*eh), 1, packet);
            fwrite(ih, sizeof(*ih), 1, packet);
            fwrite(uh, sizeof(*uh), 1, packet);
            fwrite(zh, sizeof(*zh), 1, packet);

            if(command_num % 2 != 0)
            {

                if(!lines[5])
                {
                    fprintf(stderr, "Incomplete information for status packet.\n");
                    return(1);
                }

                command_num = htonl(command_num) >> 16;
                fwrite(&command_num , 2, 1, packet);

                if(strstr(lines[5], "Add") != NULL)
                {
                    int addFlag = htonl(42);
                    fwrite(&addFlag, 2, 1, packet);
                    int groupId = get_value(lines[5]);
                    fwrite(&groupId, 4, 1, packet);
                }
                else if(strstr(lines[5], "Remove") != NULL)
                {
                    int removeFlag = 0;
                    fwrite(&removeFlag, 2, 1, packet);
                    int groupId = get_value(lines[5]);
                    fwrite(&groupId, 4, 1, packet);
                }

                else if(strstr(lines[5], "Distance") != NULL)
                {
                    if(!lines[6])
                    {
                        fprintf(stderr, "Incomplete information for GOTO packet.\n");
                        return(1);
                    }
                    unsigned int distance = get_value(lines[5]);
                    fwrite(&distance, 2, 1, packet);
                    float bearing = get_f_value(lines[6]);
                    uint32_t bear_bin = htonl(rev_convert_32(bearing));
                    fwrite(&bear_bin, 4, 1, packet);
                }

            }

            else
            {
                command_num = htonl(command_num) >> 16;
                fwrite(&command_num , 2, 1, packet);
            }
        }

        else if (zerg_type == 3)
        {
            int p_len = 54 + sizeof(struct GPS);
            int total_len = htonl(p_len)>>24;
            int ip_len = 40 + sizeof(struct GPS);
            (*uh).Len = 8 + sizeof(struct GPS);
            (*zh).TotalLen = htonl(sizeof(struct GPS))>>8;
            (*ph).PackLen = total_len;
            (*ph).DataLen = total_len;
            (*ih).TotalLen = htonl(ip_len)>>16;//Length of packet. 48 + payload
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
            free(packets);
            return(1);
        }



        free(ph);
        free(eh);
        free(ih);
        free(uh);
        free(zh);

        array_free(lines, linecount);

    }
    free(packets);
    fclose(packet);
    return(0);
}
