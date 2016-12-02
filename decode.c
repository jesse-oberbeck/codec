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
        return (1);
    }

    char *file = argv[1];
    FILE *words = fopen(file, "rb");
    int result = 0;
    int end_pos = fileSize(words);

    //Reads in file header once.
    struct FileHeader *fh = calloc(sizeof(*fh), 1);

    fread(fh, sizeof(struct FileHeader), 1, words);
    //printf("LLT: %x\n", fh->MajorVer);
    free(fh);

    int current_pos = ftell(words);

    struct ZergHeader *zh = calloc(sizeof(*zh), 1);
    struct Container *c = calloc(sizeof(*c), 1);

    //Loop to handle individual packets begins here.
    while (current_pos != end_pos)
    {
        result = processFile(words);
        if (result < 0)
        {
            free(zh);
            free(c);
            fclose(words);
            return (0);
        }

        processZergHeader(words, zh, c);
        int zergType = c->zergType;
        int totalLen = (ntohl(zh->TotalLen) >> 8) - 12;

        //printf("Zerg Len: %d\n", total_len);
        if (zergType == 0)
        {
            char *message = calloc(totalLen + 1, 1);

            fread(message, totalLen, 1, words);
            printf("%s\n", message);
            free(message);
        }

        if (zergType == 1)
        {
            zerg1Decode(words, zh);
        }

        if (zergType == 2)
        {
            zerg2Decode(words);
        }

        if (zergType == 3)
        {
            zerg3Decode(words);
        }

        if (result > 0)
        {
            fseek(words, result, SEEK_CUR);
        }
        current_pos = ftell(words);
        puts("~");
    }
    free(zh);
    free(c);
    fclose(words);
    return (0);
}
