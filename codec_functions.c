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
getValue(
    char *string)
{
/*Returns the integer value contained in a string
by first removing all alpha characters, and colons.*/

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
getDValue(
    char *string)
{
/*Returns the double value contained in a string
by first removing all alpha characters, slashes and colons.*/

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
getFValue(
    char *string)
{
/*Returns the float value contained in a string
by first removing all alpha characters, slashes and colons.*/

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
processFile(
    FILE * words)
{
/*Reads all headers prior to the Zerg header and payload, returning
a zero if there is no padding, or the number of padding bytes.*/

    struct PcapHeader *ph = calloc(sizeof(*ph), 1); //pcap header
    struct EthernetHeader *eh = calloc(sizeof(*eh), 1); //ethernet header
    struct Ipv4Header *ih = calloc(sizeof(*ih), 1); //ip header
    struct UdpHeader *uh = calloc(sizeof(*uh), 1);  //udp header

    fread(ph, sizeof(struct PcapHeader), 1, words);
    fread(eh, sizeof(struct EthernetHeader), 1, words);
    fread(ih, sizeof(struct Ipv4Header), 1, words);
    fread(uh, sizeof(struct UdpHeader), 1, words);

    int length_of_data = ntohl(ph->DataLen) >> 24;
    int ip_len = ntohl(ih->TotalLen) >> 16;

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
zerg1Decode(
    FILE * words,
    struct ZergHeader *zh)
{
/*Decoder for "status" type packets. Pulls out and prints values.*/

    struct Status *st = calloc(sizeof(*st), 1);

    fread(st, sizeof(struct Status), 1, words);
    int nameLen = (ntohl(zh->TotalLen) >> 8) - sizeof(struct ZergHeader);   // - 24;

    char *message = calloc(nameLen + 1, 1);

    fread(message, nameLen, 1, words);
    printf("Name: %s\n", message);
    printf("HP: %d/%d\n", ntohl(st->HP) >> 8, ntohl(st->MaxHP) >> 8);
    int unitTypeBin = ntohl(st->Type) >> 24;

    const char *typeArray[] =
        { "Overmind", "Larva", "Cerebrate", "Overlord", "Queen", "Drone",
"Zergling", "Lurker", "Broodling", "Hydralisk", "Guardian", "Scourge", "Ultralisk",
"Mutalisk", "Defiler", "Devourer" };

    printf("Type: %s\n", typeArray[unitTypeBin]);
    printf("Armor: %d\n", ntohl(st->Armor) >> 24);

    int binSpeed = ntohl(st->Speed);
    double speed = convert32(binSpeed);

    printf("Max Speed: %fm/s\n", speed);
    free(st);
    free(message);

}

char *
extract(
    char *line)
{
/*Pulls out and returns the string after the
colon that appears in every line of output.*/

    //Extract from line of file.
    int i = 0;
    char *name = line;
    char *splitName = strtok(name, ": /");

    while (splitName != NULL)
    {
        if (i == 1)
        {
            return (splitName);
        }
        ++i;
        splitName = strtok(NULL, ": ");
    }
    return (name);
}

void
zerg1Encode(
    char **lines,
    FILE * packet)
{
/*Encoder for status type packets. Extracts values from a
properly formatted set of values in the user given file.*/

    if (strstr(lines[4], "Name") == NULL)
    {
        fprintf(stderr, "Incorrect packet instruction.\n");
        exit(1);
    }
    if (strstr(lines[5], "HP") == NULL)
    {
        fprintf(stderr, "Incorrect packet instruction.\n");
        exit(1);
    }
    if (strstr(lines[6], "Type") == NULL)
    {
        fprintf(stderr, "Incorrect packet instruction.\n");
        exit(1);
    }
    if (strstr(lines[7], "Armor") == NULL)
    {
        fprintf(stderr, "Incorrect packet instruction.\n");
        exit(1);
    }
    if (strstr(lines[8], "Max Speed") == NULL)
    {
        fprintf(stderr, "Incorrect packet instruction.\n");
        exit(1);
    }


    struct Status *st = calloc(sizeof(*st), 1);
    char *name = extract(lines[4]);

    char *totalHp = extract(lines[5]);
    int remaining_hp = getValue(totalHp);
    int maxHp = getValue(extract(totalHp));

    st->HP = htonl(remaining_hp) >> 8;
    st->MaxHP = htonl(maxHp) >> 8;
    char *unitType = extract(lines[6]);

    const char *typeArray[] =
        { "Overmind", "Larva", "Cerebrate", "Overlord", "Queen", "Drone",
"Zergling", "Lurker", "Broodling", "Hydralisk", "Guardian", "Scourge", "Ultralisk",
"Mutalisk", "Defiler", "Devourer" };
    int type = 0;

    for (int i = 0; i < 16; ++i)
    {
        if (strcmp(typeArray[i], unitType) == 0)
        {
            type = i;
            break;
        }
    }
    st->Type = htonl(type) >> 24;

    int armor = getValue(lines[7]);

    st->Armor = htonl(armor) >> 24;

    float speed = getFValue(lines[8]);
    uint32_t packSpeed = htonl(reverseConvert32(speed));

    st->Speed = packSpeed;

    fwrite(st, 12, 1, packet);
    fwrite(name, strlen(name), 1, packet);
    free(st);
    return;

}

void
zerg2Decode(
    FILE * words)
{
/*Decoder for Command type packets. Pulls corresponding
data from pcap file, and prints it.*/
    struct Command *cm = calloc(sizeof(*cm), 1);

    fread(cm, sizeof(struct Command), 1, words);
    long command = ntohl(cm->Command) >> 16;

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

        uint32_t bearingBin;

        fread(&bearingBin, 4, 1, words);
        bearingBin = ntohl(bearingBin);
        float bearing = convert32(bearingBin);

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
        unsigned int *filler = calloc(4, 1);
        fread(filler, 2, 1, words);
        unsigned int *sequence = calloc(4, 1);
        fread(sequence, 4, 1, words);
        free(filler);
        printf("Sequence: %d\n", *sequence);
        free(sequence);
        break;
    }


    free(cm);
}

int
zerg2Encode(
    char **lines)
{
/*Encoder for Status type packets. Matches strings found in
the packet, and returns the command number associated so that
it may be written to the pcap file.*/
    struct Command *cm = calloc(sizeof(*cm), 1);
    char *comm = lines[4];
    int commandNum = 0;

    if ((strcmp(comm, "GET_STATUS") == 0) ||
        (strcmp(comm, "GET_STATUS\n") == 0))
    {
    }

    else if ((strcmp(comm, "GOTO") == 0) || (strcmp(comm, "GOTO\n") == 0))
    {
        commandNum = 1;
    }

    else if ((strcmp(comm, "GET_GPS") == 0) || (strcmp(comm, "GET_GPS\n") == 0))
    {
        commandNum = 2;
    }

    else if ((strcmp(comm, "RESERVED") == 0) || (strcmp(comm, "RESERVED\n") == 0))
    {
        commandNum = 3;
    }

    else if ((strcmp(comm, "RETURN") == 0) || (strcmp(comm, "RETURN\n") == 0))
    {
        commandNum = 4;
    }

    else if ((strcmp(comm, "SET_GROUP") == 0) || (strcmp(comm, "SET_GROUP\n") == 0))
    {
        commandNum = 5;
    }

    else if ((strcmp(comm, "STOP") == 0) || (strcmp(comm, "STOP\n") == 0))
    {
        commandNum = 6;
    }

    else if ((strcmp(comm, "REPEAT") == 0) || (strcmp(comm, "REPEAT\n") == 0))
    {
        commandNum = 7;
    }

    else
    {
        free(cm);
        return(-1);
    }

    free(cm);
    return (commandNum);
}

void
zerg3Decode(
    FILE * words)
{
/*Decoder for GPS type packets. Pulls corresponding data from
packet, and prints it.*/
    struct GPS *gps = calloc(sizeof(*gps), 1);

    fread(gps, sizeof(struct GPS), 1, words);


    double latitude = convert64(be64toh(gps->Latit));
    double longitude = convert64(be64toh(gps->Longit));

    if (latitude > 0)
    {
        printf("Latitude: %.9f deg. N\n", latitude);
    }
    else
    {
        printf("Latitude: %.9f deg. S\n", fabs(latitude));
    }

    if (longitude > 0)
    {
        printf("Longitude: %.9f deg. E\n", longitude);
    }
    else
    {
        printf("Longitude: %.9f deg. W\n", fabs(longitude));
    }

    uint32_t altitudeBin = ntohl(gps->Altit);
    float altitude = convert32(altitudeBin);

    uint32_t bearingBin = ntohl(gps->Bearing);
    float bearing = convert32(bearingBin);

    uint32_t speedBin = ntohl(gps->Speed);
    float speed = convert32(speedBin);

    uint32_t accuracyBin = ntohl(gps->Acc);
    float accuracy = convert32(accuracyBin);

    printf("Altitude: %.1fm\n", altitude * 1.8288); //Multiplying by 1.8288 to convert fathoms to meters.
    printf("Bearing: %f deg\n", bearing);
    printf("Speed: %.0fkm/h\n", speed * 3.6);   //3.6 to convert m/s to km/h.
    printf("Accuracy: %.0fm\n", accuracy);
    free(gps);
}

void
zerg3Encode(
    char **lines,
    FILE * packet)
{
/*Encoder for GPS type packet. First checks for presence
of correct lines in encode file, then extracts values
and writes them to the pcap file.*/
    struct GPS *gps = calloc(sizeof(*gps), 1);

    if (strstr(lines[4], "Latitude") == NULL)
    {
        fprintf(stderr, "Incorrect packet instruction.\n");
        exit(1);
    }
    if (strstr(lines[5], "Longitude") == NULL)
    {
        fprintf(stderr, "Incorrect packet instruction.\n");
        exit(1);
    }
    if (strstr(lines[6], "Altitude") == NULL)
    {
        fprintf(stderr, "Incorrect packet instruction.\n");
        exit(1);
    }
    if (strstr(lines[7], "Bearing") == NULL)
    {
        fprintf(stderr, "Incorrect packet instruction.\n");
        exit(1);
    }
    if (strstr(lines[8], "Speed") == NULL)
    {
        fprintf(stderr, "Incorrect packet instruction.\n");
        exit(1);
    }
    if (strstr(lines[9], "Accuracy") == NULL)
    {
        fprintf(stderr, "Incorrect packet instruction.\n");
        exit(1);
    }

    double lat = getDValue(lines[4]);
    double lon = getDValue(lines[5]);

    if (strstr(lines[4], "S") != NULL)
    {
        lat = lat * -1;
    }

    if (strstr(lines[5], "W") == NULL)
    {
        lon = lon * -1;
    }

    float alt = getFValue(lines[6]) * .546807;
    float bear = getFValue(lines[7]);
    float speed = getFValue(lines[8]) * .277778;
    float acc = getFValue(lines[9]);

    uint64_t latBin = be64toh(reverseConvert64(lat));
    uint64_t lonBin = be64toh(reverseConvert64(lon));
    uint32_t altBin = htonl(reverseConvert32(alt));
    uint32_t bearBin = htonl(reverseConvert32(bear));
    uint32_t speedBin = htonl(reverseConvert32(speed));
    uint32_t accBin = htonl(reverseConvert32(acc));

    gps->Longit = lonBin;
    gps->Latit = latBin;
    gps->Altit = altBin;
    gps->Bearing = bearBin;
    gps->Speed = speedBin;
    gps->Acc = accBin;

    fwrite(gps, 32, 1, packet);
    free(gps);
    return;
}

void
processZergHeader(
    FILE * words,
    struct ZergHeader *zh,
    struct Container *c)
{
/*Decodes and prints all packet information prior
to the Zerg Header.*/

    fread(zh, sizeof(struct ZergHeader), 1, words);
    int zergType = ntohl(zh->Type) >> 24;
    int totalLen = ntohl(zh->TotalLen) >> 8;
    if((zergType < 0) || (zergType > 3))
    {
        fprintf(stderr, "Invalid packet detected.\n");
        exit(1);
    }
    int sequence = ntohl(zh->Sequence);
    if((sequence < 0) || (sequence > 65535))
    {
        fprintf(stderr, "Invalid packet detected.\n");
        exit(1);
    }

    //printf("Zerg Version: %x\n", ntohl(zh->Version) >> 24);
    printf("Message Type: %d\n", zergType);
    printf("Sequence: %d\n", sequence);
    //printf("Zerg Packet Length: %d\n", totalLen);
    printf("Destination ID: %d\n", ntohl(zh->Did) >> 16);
    printf("Source ID: %d\n", ntohl(zh->Sid) >> 16);
    c->zergType = zergType;
    c->totalLen = totalLen;
    return;
}

int
fileSize(
    FILE * words)
{
/*Gets end of file.*/

    fseek(words, 0, SEEK_END);
    long end = ftell(words);

    fseek(words, 0, SEEK_SET);
    return (end);
}

float
convert32(
    uint32_t num)
{
/*Converts a uint32_t to a float.*/
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
reverseConvert32(
    float num)
{
/*Converts a float to a uint32_t.*/
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
convert64(
    uint64_t num)
{
/*Converts a uint64_t to a double.*/
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
reverseConvert64(
    double num)
{
/*Converts a double to a uint64_t.*/
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
