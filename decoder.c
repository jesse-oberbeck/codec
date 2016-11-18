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

//Headers are 752 total bytes for a single set.

int
process_file(
    FILE * words)
{

    struct PcapHeader *ph = calloc(sizeof(*ph), 1); //pcap header
    struct EthernetHeader *eh = calloc(sizeof(*eh), 1); //ethernet header
    struct Ipv4Header *ih = calloc(sizeof(*ih), 1); //ip header
    struct UdpHeader *uh = calloc(sizeof(*uh), 1);  //udp header



    int fail = fread(ph, sizeof(struct PcapHeader), 1, words);
    if(fail == 0)
    {
        return(-1);
    }
    
    fread(eh, sizeof(struct EthernetHeader), 1, words);
    fread(ih, sizeof(struct Ipv4Header), 1, words);
    fread(uh, sizeof(struct UdpHeader), 1, words);

    /*  Printing Header Information  */
    int length_of_data = htonl(ph->DataLen) >> 24;
    int ip_len = htonl(ih->TotalLen) >> 16;
    printf("%d - %d \n", length_of_data, ip_len);
    if(length_of_data == 0){
    printf("Length of Data Captured is %d.\nEmpty file.\n", length_of_data);
    }
    //int eop = ph->PackLen;////////////////////
    //printf("EOP: %d\n", eop);


    free(ph);
    free(eh);
    free(ih);
    free(uh);

    return(length_of_data - ip_len - 14);
}

struct Container
{
    int zerg_type;
    int total_len;
};

void
process_zerg_header(
    FILE * words,
    struct ZergHeader *zh,
    struct Container *c)
{
    fread(zh, sizeof(struct ZergHeader), 1, words);
    int zerg_type = htonl(zh->Type) >> 24;
    int total_len = htonl(zh->TotalLen) >> 8;

    //printf("Zerg Version: %x\n", htonl(zh->Version) >> 24);
    printf("Message Type: %d\n", zerg_type);
    printf("Sequence: %d\n", htonl(zh->Sequence));
    printf("Zerg Packet Length: %d\n", total_len);
    printf("Destination ID: %d\n", htonl(zh->Did) >> 16);
    printf("Source ID: %d\n", htonl(zh->Sid) >> 16);
    c->zerg_type = zerg_type;
    c->total_len = total_len;
    return;
}

/*Get end of file */
int
file_size(
    FILE * words)
{

    fseek(words, 0, SEEK_END);
    long end = ftell(words);

    fseek(words, 0, SEEK_SET);
    return (end);
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

const char * translate_type(int type)
{
    switch(type)
    {
    case(0):
        return("Overlord");
    case(1):
        return("Larva");
    case(2):
        return("Cerebrate");
    case(6):
        return("Zergling");
    }
    return("Invalid");
}

int
main(
    int argc,
    char *argv[])
{
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
    struct FileHeader *fh = calloc(sizeof(*fh), 1); //file header
    fread(fh, sizeof(struct FileHeader), 1, words);
    free(fh);

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
    int current_pos = ftell(words);

    struct ZergHeader *zh = calloc(sizeof(*zh), 1);
    struct Container *c = calloc(sizeof(*c), 1);
    while(current_pos != end_pos)
    {
        puts("LOOP");
        result = process_file(words);
        if(result <= 0)
        {
            printf("END\n");
            return(0);
        }
        process_zerg_header(words, zh, c);
        int zerg_type = c->zerg_type;
        int total_len = (htonl(zh->TotalLen) >> 8) - 12;
        printf("namelen: %d\n", total_len);
        if (zerg_type == 0)
        {
            char *message = calloc(total_len, 1);

            fread(message, total_len, 1, words);
            printf("%s\n", message);
            free(message);
        }

        if (zerg_type == 1)
        {
            struct Status *st = calloc(sizeof(*st), 1);

            fread(st, sizeof(struct Status), 1, words);
            int nameLen = (htonl(zh->TotalLen) >> 8) - 24;

            printf("namelen: %d\n", nameLen);
            char *message = calloc(nameLen, 1);

            fread(message, nameLen, 1, words);
            printf("Name: %s\n", message);
            printf("HP: %d/%d\n", htonl(st->HP) >> 8, htonl(st->MaxHP) >> 8);
            int unit_type_bin = htonl(st->Type) >> 24;
            //const char *unit_type = translate_type(unit_type_bin);
            
            const char *type_array[] = {"Overmind", "Larva", "Cerebrate", "Overlord", "Queen", "Drone", "Zergling", "Lurker", "Broodling", "Hydralisk", "Guardian", "Scourge", "Ultralisk", "Mutalisk", "Defiler", "Devourer"};
            
            printf("Type: %s\n", type_array[unit_type_bin]);
            printf("Armor: %d\n", htonl(st->Armor) >> 8);

            int bin_speed = htonl(st->Speed);
            double speed = convert_32(bin_speed);

            printf("Max Speed: %fm/s\n", speed);
            free(st);
            free(message);

        }

        if (zerg_type == 2)
        {
            struct Command *cm = calloc(sizeof(*cm), 1);

            fread(cm, sizeof(struct Command), 1, words);
            int command = htonl(cm->Command);

            switch (command)
            {
            case (0):
                printf("GET_STATUS\n");
                break;
            case (1):
                printf("GOTO\n");
                break;
            case (2):
                printf("GET_GPS\n");
                break;
            case (3):
                printf("RESERVED\n");
                break;
            case (4):
                printf("RETURN\n");
                break;
            case (5):
                printf("SET_GROUP\n");
                break;
            case (6):
                printf("STOP\n");
                break;
            case (7):
                printf("REPEAT\n");
                break;
            }
            printf("P1: %d\n", htonl(cm->Param1));
            if (command % 2 == 0)
            {
                cm = realloc(cm, sizeof(cm) - 4);
                //fseek(words, -4, SEEK_CUR);

            }
            else
            {
                printf("P2: %d\n", htonl(cm->Param2));
            }

            free(cm);
        }

        if (zerg_type == 3)
        {
            struct GPS *gps = calloc(sizeof(*gps), 1);

            fread(gps, sizeof(struct GPS), 1, words);


            double latitude = convert_64(be64toh(gps->Latit));
            double longitude = convert_64(be64toh(gps->Longit));

            if (latitude > 0)
            {
                printf("latitude: %.9f deg. N\n", latitude);
            }
            else
            {
                printf("latitude: %.9f deg. S\n", fabs(latitude));
            }

            if (longitude > 0)
            {
                printf("longitude: %.9f deg. E\n", longitude);
            }
            else
            {
                printf("longitude: %.9f deg. W\n", fabs(longitude));
            }

            uint32_t altitude_bin = htonl(gps->Altit);
            float altitude = convert_32(altitude_bin);

            uint32_t bearing_bin = htonl(gps->Bearing);
            float bearing = convert_32(bearing_bin);

            uint32_t speed_bin = htonl(gps->Speed);
            float speed = convert_32(speed_bin);

            uint32_t acc_bin = htonl(gps->Acc);
            float accuracy = convert_32(acc_bin);

            printf("Altitude: %.1fm\n", altitude * 1.8288); //Multiplying by 1.8288 to convert fathoms to meters.
            printf("Bearing: %f deg\n", bearing);
            printf("Speed: %.0fkm/h\n", speed * 3.6);   //3.6 to convert m/s to km/h.
            printf("Accuracy: %.0fm\n", accuracy);
        }
    current_pos = ftell(words);
    int padding = result;
    printf("RESULT: %d\n", padding);
    fseek(words, padding, SEEK_CUR);
    puts("");
    }
    free(zh);
    free(c);
    fclose(words);

}
