#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <ctype.h>
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

int main(void)
{
    //char *testheader = "\xD4\xC3\xB2\xA1";
    //FILE *words = fopen("testing.pcap", "wb+");
    //fwrite(testheader, strlen(testheader), 1, words);
    
    char test[] = "check 123";
    int val = get_value(test);
    printf("val: %d\n", val);
}
