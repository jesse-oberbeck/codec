
struct Container
{
    int zerg_type;
    int total_len;
};


struct __attribute__ ((__packed__)) FileHeader  //stackoverflow.com/questions/4306186/structure-padding-and-packing
{
    int FileType:32;
    int MajorVer:16;
    int MinorVer:16;
    int GMT:32;
    int Acc:32;
    int MaxLen:32;
    int LLT:32;
};

struct __attribute__ ((__packed__)) PcapHeader
{
    int Epoch:32;
    int EpochMil:32;
    int DataLen:32;
    int PackLen:32;
};

struct __attribute__ ((__packed__)) EthernetHeader
{
    int Dmac:32;
    int Dmac2:16;
    int Smac:32;
    int Smac2:16;
    int Etype:16;
};

struct __attribute__ ((__packed__)) Ipv4Header
{
    int IHL:4;
    int Version:4;
    int DSCP:6;
    int ECN:2;
    int TotalLen:16;
    int Ident:16;
    int Flags:3;
    int FragOff:13;
    int TTL:8;
    int Protocol:8;
    int CheckSum:16;
    int SIP:32;
    int DIP:32;
};

struct __attribute__ ((__packed__)) UdpHeader
{
    int Sport:16;
    int Dport:16;
    int Len:16;
    int CheckSum:16;
};

struct __attribute__ ((__packed__)) ZergHeader
{
    int Type:4;
    int Version:4;
    int TotalLen:24;
    int Sid:16;
    int Did:16;
    int Sequence:32;
};

struct Message
{
    char *Message;
    //Might not be needed. Entire payload is text.
    //"Not null terminated" so will have to cut off at packet len.
};

struct __attribute__ ((__packed__)) Status
{
    int HP:24;
    int Armor:8;
    int MaxHP:24;
    int Type:8;
    int Speed:32;

    //uint32_t Name   : 32;
};

struct __attribute__ ((__packed__)) Command
{
    int Command:16;
    //int Param1:16;
    //int Param2:32;
};

struct __attribute__ ((__packed__)) GPS
{
    uint64_t Longit;
    uint64_t Latit;
    int Altit:32;
    int Bearing:32;
    int Speed:32;
    int Acc:32;
};

int
process_file(
    FILE * words);
    
void
process_zerg_header(
    FILE * words,
    struct ZergHeader *zh,
    struct Container *c);

int
file_size(
    FILE * words);

float
convert_32(
    uint32_t num);

double
convert_64(
    uint64_t num);

void zerg1(FILE *words, struct ZergHeader *zh);
void zerg2(FILE *words);
void zerg3(FILE *words);
