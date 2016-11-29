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
    int result = 0;
    int end_pos = file_size(words);

//Reads in file header once.
    struct FileHeader *fh = calloc(sizeof(*fh), 1);
    fread(fh, sizeof(struct FileHeader), 1, words);
    //printf("LLT: %x\n", fh->MajorVer);
    free(fh);

    int current_pos = ftell(words);

    struct ZergHeader *zh = calloc(sizeof(*zh), 1);
    struct Container *c = calloc(sizeof(*c), 1);

//Loop to handle individual packets begins here.
    while(current_pos != end_pos)
    {
        result = process_file(words);
        if(result < 0)
        {
            free(zh);
            free(c);
            fclose(words);
            return(0);
        }

        process_zerg_header(words, zh, c);
        int zerg_type = c->zerg_type;
        int total_len = (htonl(zh->TotalLen) >> 8) - 12;
        //printf("Zerg Len: %d\n", total_len);
        if (zerg_type == 0)
        {
            char *message = calloc(total_len, 1);

            fread(message, total_len, 1, words);
            printf("%s\n", message);
            free(message);
        }

        if (zerg_type == 1)
        {
            zerg1_decode(words, zh);
        }

        if (zerg_type == 2)
        {
            zerg2_decode(words);
        }

        if (zerg_type == 3)
        {
            zerg3_decode(words);
        }

    current_pos = ftell(words);
    //printf("RESULT: %d\n", result);
    if(result > 0){
        fseek(words, result, SEEK_CUR);
    }
    puts("~");
    }
    free(zh);
    free(c);
    fclose(words);
    return(0);

}
