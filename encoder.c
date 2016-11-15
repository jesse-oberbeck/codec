#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>

int main(void)
{
    char *testheader = "\xD4\xC3\xB2\xA1";
    FILE *words = fopen("testing.pcap", "wb+");
    fwrite(testheader, strlen(testheader), 1, words);
}
