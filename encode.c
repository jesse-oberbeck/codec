#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <endian.h>
#include <arpa/inet.h>
#include "structures.h"

int
main(
    int argc,
    char *argv[])
{
    //Check for file name provided as arg.
    if (argc < 3)
    {
        fprintf(stderr, "Please provide source and destination file names.\n");
        return (1);
    }
    else if (access(argv[1], F_OK) == -1)
    {
        fprintf(stderr, "Invalid file name.\n");
        return (1);
    }
    int packetcount = 0;
    int linecount = 0;
    char **packets = initialize(&packetcount, argv[1]);
    FILE *packet = fopen(argv[2], "wb+");
    struct FileHeader *fh = calloc(sizeof(*fh), 1);

    (*fh).FileType = htonl((unsigned int) 3569595041);  //3569595041 = "\xD4\xC3\xB2\xA1"
    (*fh).MajorVer = 2;
    (*fh).MinorVer = 4;
    (*fh).MaxLen = 65535;
    (*fh).LLT = 1;
    fwrite(fh, sizeof(*fh), 1, packet);
    free(fh);

    for (int i = 0; i < packetcount; ++i)
    {
        linecount = 0;
        char **lines = setup(&linecount, packets[i]);

        --linecount;
        int zerg_type = getValue(lines[0]);
        int did = getValue(lines[2]);
        int sid = getValue(lines[3]);

        struct PcapHeader *ph = calloc(sizeof(*ph), 1); //pcap header
        struct EthernetHeader *eh = calloc(sizeof(*eh), 1); //ethernet header
        struct Ipv4Header *ih = calloc(sizeof(*ih), 1); //ip header
        struct UdpHeader *uh = calloc(sizeof(*uh), 1);  //udp header
        struct ZergHeader *zh = calloc(sizeof(*zh), 1);

        //Values that are the same in each packet, hard-coded.
        (*eh).Etype = 2048;
        (*ih).Version = '\x4';
        (*ih).IHL = '\x5';
        (*ih).Protocol = 17;
        (*uh).Sport = htonl(8);
        (*uh).Dport = htonl(4276);
        (*zh).Version = '\x1';
        (*zh).Type = zerg_type;
        (*zh).Sid = htonl(sid) >> 16;
        (*zh).Did = htonl(did) >> 16;

        if (!packet)
        {
            fprintf(stderr, "Failed to open file!");
            free(ph);
            free(eh);
            free(ih);
            free(uh);
            free(zh);
            return (1);
        }


        //Encode message packet.
        if (zerg_type == 0)
        {
            int zerglen = 12 + strlen(lines[4]);    //Length of Zerg Header + length of message.
            int p_len = 42 + zerglen;
            int total_len = htonl(p_len) >> 24;
            int ip_len = 28 + zerglen;

            (*uh).Len = htonl(8 + zerglen); //Add 8 for length of UDP header.
            (*ph).PackLen = total_len;
            (*ph).DataLen = total_len;
            (*ih).TotalLen = htonl(ip_len) >> 16;   //Length of packet. 48 + payload
            (*zh).TotalLen = htonl(zerglen) >> 8;
            fwrite(ph, sizeof(*ph), 1, packet);
            fwrite(eh, sizeof(*eh), 1, packet);
            fwrite(ih, sizeof(*ih), 1, packet);
            fwrite(uh, sizeof(*uh), 1, packet);
            fwrite(zh, sizeof(*zh), 1, packet);
            fwrite(lines[4], strlen(lines[4]), 1, packet);
        }

        //Encode Status packet.
        else if (zerg_type == 1)
        {
            int zerglen = (strlen(lines[4]) - 6);
            int p_len = 54 + sizeof(struct Status) + zerglen;   //54 is the sum of bytes prior to payload.
            int total_len = htonl(p_len) >> 24;
            int ip_len = 40 + sizeof(struct Status) + zerglen;

            (*uh).Len = htonl(8 + zerglen);
            (*ph).PackLen = total_len;
            (*ph).DataLen = total_len;
            (*ih).TotalLen = htonl(ip_len) >> 16;   //Length in IPv4 Header. 48 + payload.
            int zergHeaderLenth = zerglen + 12;

            (*zh).TotalLen = htonl(zergHeaderLenth) >> 8;
            fwrite(ph, sizeof(*ph), 1, packet);
            fwrite(eh, sizeof(*eh), 1, packet);
            fwrite(ih, sizeof(*ih), 1, packet);
            fwrite(uh, sizeof(*uh), 1, packet);
            fwrite(zh, sizeof(*zh), 1, packet);
            zerg1Encode(lines, packet);
        }

        //Encode Command packet.
        else if (zerg_type == 2)
        {
            int command_num = zerg2Encode(lines);
            int p_len = 0;
            int total_len = 0;
            int ip_len = 0;
            int udpLength = 0;

            if (command_num < 0)
            {
                fprintf(stderr, "Invalid command.");
            }

            if (command_num % 2 == 0)
            {
                p_len = 54 + 2; //54 is the sum of bytes prior to payload.
                udpLength = 20 + 2;
                total_len = htonl(p_len) >> 24;
                ip_len = 40 + 2;    //Length in IPv4 Header. 48 + payload.
            }
            else
            {
                p_len = 54 + 8; //54 is the sum of bytes prior to payload.
                udpLength = 20 + 8;
                total_len = htonl(p_len) >> 24;
                ip_len = 40 + 8;    //Length in IPv4 Header. 48 + payload.
            }

            (*zh).TotalLen = htonl(udpLength - 8) >> 8;
            (*ph).PackLen = total_len;
            (*ph).DataLen = total_len;
            (*uh).Len = htonl(udpLength);
            (*ih).TotalLen = htonl(ip_len) >> 16;   //Length of packet. 48 + payload
            //struct Command *zp = calloc(sizeof(*zp), 1);
            fwrite(ph, sizeof(*ph), 1, packet);
            fwrite(eh, sizeof(*eh), 1, packet);
            fwrite(ih, sizeof(*ih), 1, packet);
            fwrite(uh, sizeof(*uh), 1, packet);
            fwrite(zh, sizeof(*zh), 1, packet);

            //Decide whether or not to include parameters 1 and 2.
            if (command_num % 2 != 0)
            {

                if (!lines[5])
                {
                    fprintf(stderr,
                            "Incomplete information for status packet.\n");
                    return (1);
                }

                command_num = htonl(command_num) >> 16;
                fwrite(&command_num, 2, 1, packet);

                if (strstr(lines[5], "Add") != NULL)
                {
                    int addFlag = htonl(42);

                    fwrite(&addFlag, 2, 1, packet);
                    int groupId = getValue(lines[5]);

                    fwrite(&groupId, 4, 1, packet);
                }
                else if (strstr(lines[5], "Remove") != NULL)
                {
                    int removeFlag = 0;

                    fwrite(&removeFlag, 2, 1, packet);
                    int groupId = getValue(lines[5]);

                    fwrite(&groupId, 4, 1, packet);
                }

                else if (strstr(lines[5], "Distance") != NULL)
                {
                    if (!lines[6])
                    {
                        fprintf(stderr,
                                "Incomplete information for GOTO packet.\n");
                        return (1);
                    }
                    unsigned int distance = getValue(lines[5]);

                    fwrite(&distance, 2, 1, packet);
                    float bearing = getFValue(lines[6]);
                    uint32_t bear_bin = htonl(reverseConvert32(bearing));

                    fwrite(&bear_bin, 4, 1, packet);
                }

                else if (strstr(lines[5], "Sequence") != NULL)
                {
                    unsigned int sequence = getValue(lines[5]);

                    fwrite(&sequence, 2, 1, packet);
                    fwrite(&sequence, 4, 1, packet);

                }

                else
                {
                    fprintf(stderr, "Incorrect packet information.\n");
                    return (1);
                }

            }

            else
            {
                command_num = htonl(command_num) >> 16;
                fwrite(&command_num, 2, 1, packet);
            }
        }

        //Encode GPS packet.
        else if (zerg_type == 3)
        {
            int p_len = 54 + sizeof(struct GPS);    //54 is the sum of bytes prior to payload.
            int total_len = htonl(p_len) >> 24;
            int ip_len = 40 + sizeof(struct GPS);   //Length in IPv4 Header. 48 + payload.

            (*uh).Len = 8 + sizeof(struct GPS);
            (*zh).TotalLen =
                htonl(sizeof(struct GPS) + sizeof(struct ZergHeader)) >> 8;
            (*ph).PackLen = total_len;
            (*ph).DataLen = total_len;
            (*ih).TotalLen = htonl(ip_len) >> 16;
            fwrite(ph, sizeof(*ph), 1, packet);
            fwrite(eh, sizeof(*eh), 1, packet);
            fwrite(ih, sizeof(*ih), 1, packet);
            fwrite(uh, sizeof(*uh), 1, packet);
            fwrite(zh, sizeof(*zh), 1, packet);
            zerg3Encode(lines, packet);
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
            arrayFree(lines, linecount);
            free(packets);
            return (1);
        }



        free(ph);
        free(eh);
        free(ih);
        free(uh);
        free(zh);

        arrayFree(lines, linecount);

    }
    free(packets);
    fclose(packet);
    return (0);
}
