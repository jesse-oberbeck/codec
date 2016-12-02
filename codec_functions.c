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
#include <ctype.h>
#include "structures.h"

int
get_value(
    char *string)
{
    int len = strlen(string);

    for (int i = 0; i < len; ++i)
    {
        if ((isalpha(string[i]) != 0) || string[i] == ':')
        {
            string[i] = ' ';
        }
    }
    int value = strtol(string, NULL, 10);

    return (value);
}

double
get_d_value(
    char *string) 
{
    int len = strlen(string);

    for (int i = 0; i < len; ++i)
    {
        if ((isalpha(string[i]) != 0) || string[i] == ':' || string[i] == '/')
        {
            string[i] = ' ';
        }
    }
    double value = strtod(string, NULL);

    return (value);
}

float
get_f_value(
    char *string)
{
    int len = strlen(string);

    for (int i = 0; i < len; ++i)
    {
        if ((isalpha(string[i]) != 0) || string[i] == ':' || string[i] == '/')
        {
            string[i] = ' ';
        }
    }
    float value = strtof(string, NULL);

    return (value);
}

int
process_file(
    FILE * words)
{

    struct PcapHeader *ph = calloc(sizeof(*ph), 1); //pcap header
    struct EthernetHeader *eh = calloc(sizeof(*eh), 1); //ethernet header
    struct Ipv4Header *ih = calloc(sizeof(*ih), 1); //ip header
    struct UdpHeader *uh = calloc(sizeof(*uh), 1);  //udp header

    fread(ph, sizeof(struct PcapHeader), 1, words);
    fread(eh, sizeof(struct EthernetHeader), 1, words);
    fread(ih, sizeof(struct Ipv4Header), 1, words);
    fread(uh, sizeof(struct UdpHeader), 1, words);

    int length_of_data = htonl(ph->DataLen) >> 24;
    int ip_len = htonl(ih->TotalLen) >> 16;

    if (length_of_data <= 0)
    {
        return (-1);
    }

    free(ph);
    free(eh);
    free(ih);
    free(uh);

    return (length_of_data - ip_len - 14);
}

void
zerg1_decode(
    FILE * words,
    struct ZergHeader *zh)
{
    struct Status *st = calloc(sizeof(*st), 1);

    fread(st, sizeof(struct Status), 1, words);
    int nameLen = (htonl(zh->TotalLen) >> 8);   // - 24;

    char *message = calloc(nameLen + 1, 1);

    fread(message, nameLen, 1, words);
    printf("Name: %s\n", message);
    printf("HP: %d/%d\n", htonl(st->HP) >> 8, htonl(st->MaxHP) >> 8);
    int unit_type_bin = htonl(st->Type) >> 24;

    const char *type_array[] =
        { "Overmind", "Larva", "Cerebrate", "Overlord", "Queen", "Drone",
"Zergling", "Lurker", "Broodling", "Hydralisk", "Guardian", "Scourge", "Ultralisk",
"Mutalisk", "Defiler", "Devourer" };

    printf("Type: %s\n", type_array[unit_type_bin]);
    printf("Armor: %d\n", htonl(st->Armor) >> 8);

    int bin_speed = htonl(st->Speed);
    double speed = convert_32(bin_speed);

    printf("Max Speed: %fm/s\n", speed);
    free(st);
    free(message);

}

char *
extract(
    char *line)
{
    //Extract from line of file.
    int i = 0;
    char *name = line;
    char *split_name = strtok(name, ": /");

    while (split_name != NULL)
    {
        if (i == 1)
        {
            return (split_name);
        }
        ++i;
        split_name = strtok(NULL, ": ");
    }
    return (name);
}

void
zerg1_encode(
    char **lines,
    FILE * packet)
{
    struct Status *st = calloc(sizeof(*st), 1);
    char *name = extract(lines[4]);

    char *total_hp = extract(lines[5]);
    int remaining_hp = get_value(total_hp);
    int max_hp = get_value(extract(total_hp));

    st->HP = htonl(remaining_hp) >> 8;
    st->MaxHP = htonl(max_hp) >> 8;
    char *unit_type = extract(lines[6]);

    const char *type_array[] =
        { "Overmind", "Larva", "Cerebrate", "Overlord", "Queen", "Drone",
"Zergling", "Lurker", "Broodling", "Hydralisk", "Guardian", "Scourge", "Ultralisk",
"Mutalisk", "Defiler", "Devourer" };
    int type = 0;

    for (int i = 0; i < 16; ++i)
    {
        if (strcmp(type_array[i], unit_type) == 0)
        {
            type = i;
            break;
        }
    }
    st->Type = htonl(type) >> 24;

    int armor = get_value(lines[7]);

    st->Armor = htonl(armor) >> 8;

    float speed = get_f_value(lines[8]);
    uint32_t pack_speed = htonl(rev_convert_32(speed));

    st->Speed = pack_speed;

    fwrite(st, 12, 1, packet);
    fwrite(name, strlen(name), 1, packet);
    free(st);
    return;

}

void
zerg2_decode(
    FILE * words)
{
    struct Command *cm = calloc(sizeof(*cm), 1);

    fread(cm, sizeof(struct Command), 1, words);
    long command = htonl(cm->Command) >> 16;

    switch (command)
    {
    case (0):
        printf("GET_STATUS\n");
        break;
    case (1):
        printf("GOTO\n");
        unsigned int *distance = calloc(4, 1);

        fread(distance, 2, 1, words);

        printf("Distance: %d\n", *distance);

        uint32_t bearing_bin;

        fread(&bearing_bin, 4, 1, words);
        bearing_bin = ntohl(bearing_bin);
        float bearing = convert_32(bearing_bin);

        printf("Bearing: %f\n", bearing);
        free(distance);

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
        int *P1 = calloc(4, 1);

        fread(P1, 2, 1, words);
        if (*P1)
        {
            printf("Add zerg to");
        }
        else
        {
            printf("Remove zerg from");
        }
        int *P2 = calloc(4, 1);

        fread(P2, 4, 1, words);
        printf(": %d\n", *P2);
        free(P1);
        free(P2);
        break;
    case (6):
        printf("STOP\n");
        break;
    case (7):
        printf("REPEAT\n");
        break;
    }


    free(cm);
}

int
zerg2_encode(
    char **lines)
{
    struct Command *cm = calloc(sizeof(*cm), 1);
    char *comm = lines[4];
    int command_num = 0;

    if ((strcmp(comm, "GET_STATUS") == 0) ||
        (strcmp(comm, "GET_STATUS\n") == 0))
    {
    }

    if ((strcmp(comm, "GOTO") == 0) || (strcmp(comm, "GOTO\n") == 0))
    {
        command_num = 1;
    }

    if ((strcmp(comm, "GET_GPS") == 0) || (strcmp(comm, "GET_GPS\n") == 0))
    {
        command_num = 2;
    }

    if ((strcmp(comm, "RESERVED") == 0) || (strcmp(comm, "RESERVED\n") == 0))
    {
        command_num = 3;
    }

    if ((strcmp(comm, "RETURN") == 0) || (strcmp(comm, "RETURN\n") == 0))
    {
        command_num = 4;
    }

    if ((strcmp(comm, "SET_GROUP") == 0) || (strcmp(comm, "SET_GROUP\n") == 0))
    {
        command_num = 5;
    }

    if ((strcmp(comm, "STOP") == 0) || (strcmp(comm, "STOP\n") == 0))
    {
        command_num = 6;
    }

    if ((strcmp(comm, "REPEAT") == 0) || (strcmp(comm, "REPEAT\n") == 0))
    {
        command_num = 7;
    }

    free(cm);
    return (command_num);
}

void
zerg3_decode(
    FILE * words)
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
    free(gps);
}

void
zerg3_encode(
    char **lines,
    FILE * packet)
{
    struct GPS *gps = calloc(sizeof(*gps), 1);

    double lat = get_d_value(lines[4]);
    double lon = get_d_value(lines[5]);

    if (strstr(lines[4], "S") != NULL)
    {
        lat = lat * -1;
    }

    if (strstr(lines[5], "W") == NULL)
    {
        lon = lon * -1;
    }

    float alt = get_f_value(lines[6]) * .546807;
    float bear = get_f_value(lines[7]);
    float speed = get_f_value(lines[8]) * .277778;
    float acc = get_f_value(lines[9]);

    uint64_t lat_bin = be64toh(rev_convert_64(lat));
    uint64_t lon_bin = be64toh(rev_convert_64(lon));
    uint32_t alt_bin = htonl(rev_convert_32(alt));
    uint32_t bear_bin = htonl(rev_convert_32(bear));
    uint32_t speed_bin = htonl(rev_convert_32(speed));
    uint32_t acc_bin = htonl(rev_convert_32(acc));

    gps->Longit = lon_bin;
    gps->Latit = lat_bin;
    gps->Altit = alt_bin;
    gps->Bearing = bear_bin;
    gps->Speed = speed_bin;
    gps->Acc = acc_bin;

    fwrite(gps, 32, 1, packet);
    free(gps);
    return;
}

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
    //printf("Zerg Packet Length: %d\n", total_len);
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

uint32_t
rev_convert_32(
    float num)
{
    union
    {
        float f;
        uint32_t u;
    } converter;                //stackoverflow.com/questions/15685181/how-to-get-the-sign-mantissa-and-exponent-of-a-floating-point-number

    converter.f = num;
    uint32_t result = converter.u;

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

uint64_t
rev_convert_64(
    double num)
{
    union
    {
        double f;
        uint64_t u;
    } converter;                //stackoverflow.com/questions/15685181/how-to-get-the-sign-mantissa-and-exponent-of-a-floating-point-number
    uint64_t zero = 0;

    converter.u = zero;
    converter.f = num;
    uint64_t result = converter.u;

    return (result);
}
