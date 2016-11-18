#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

float
convert_32(
    uint32_t num)
{
    union
    {
        float f;
        uint32_t u;
    } converter;                //stackoverflow.com/questions/15685181/how-to-get-the-sign-mantissa-and-exponent-of-a-floating-point-number

    converter.u = num;
    float result = converter.f;

    return (result);
}

double
convert_64(
    uint64_t num)
{
    union
    {
        double f;
        uint64_t u;
    } converter;                //stackoverflow.com/questions/15685181/how-to-get-the-sign-mantissa-and-exponent-of-a-floating-point-number
    uint64_t zero = 0;

    converter.u = zero;
    converter.u = num;
    double result = converter.f;

    return (result);
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
int main(void)
{
    //char *testheader = "\xD4\xC3\xB2\xA1";
    //FILE *words = fopen("testing.pcap", "wb+");
    //fwrite(testheader, strlen(testheader), 1, words);
    
    char test[] = "check 123";
    int val = get_value(test);
    printf("val: %d\n", val);
}
