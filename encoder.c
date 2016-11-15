#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>

int main(void)
{
    char *testheader = "\xA1\xB2\xC3\xD4";
    FILE *words = fopen("testing.pcap", "wb+");
    fwrite(testheader, strlen(testheader), 1, words);
}