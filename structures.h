/*Structure for passing two pieces of data back
from a function*/
struct Container
{
    int zergType;
    int totalLen;
};

/*Structure for handling the File Header*/
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

/*Structure for handling the Pcap Header*/
struct __attribute__ ((__packed__)) PcapHeader
{
    int Epoch:32;
    int EpochMil:32;
    int DataLen:32;
    int PackLen:32;
};

/*Structure for handling the Ethernet Header*/
struct __attribute__ ((__packed__)) EthernetHeader
{
    int Dmac:32;
    int Dmac2:16;
    int Smac:32;
    int Smac2:16;
    int Etype:16;
};

/*Structure for handling the IPv4 Header*/
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

/*Structure for handling the UDP Header*/
struct __attribute__ ((__packed__)) UdpHeader
{
    int Sport:16;
    unsigned int Dport:16;
    int Len:16;
    int CheckSum:16;
};

/*Structure for handling the Zerg Header*/
struct __attribute__ ((__packed__)) ZergHeader
{
    int Type:4;
    int Version:4;
    int TotalLen:24;
    int Sid:16;
    int Did:16;
    int Sequence:32;
};

/*Structure for handling the Status type packet.*/
struct __attribute__ ((__packed__)) Status
{
    int HP:24;
    int Armor:8;
    int MaxHP:24;
    int Type:8;
    int Speed:32;

    //uint32_t Name   : 32;
};

/*Structure for handling the Command type packet.*/
struct __attribute__ ((__packed__)) Command
{
    int Command:16;
};

/*Structure for handling the GPS type packet.*/
struct __attribute__ ((__packed__)) GPS
{
    uint64_t Longit;
    uint64_t Latit;
    int Altit:32;
    int Bearing:32;
    int Speed:32;
    int Acc:32;
};


int processFile(
    FILE * words);

void processZergHeader(
    FILE * words,
    struct ZergHeader *zh,
    struct Container *c);

int fileSize(
    FILE * words);

float convert32(
    uint32_t num);

double convert64(
    uint64_t num);

uint64_t reverseConvert64(
    double num);

uint32_t reverseConvert32(
    float num);

void zerg1Decode(
    FILE * words,
    struct ZergHeader *zh);

void zerg2Decode(
    FILE * words);

void zerg3Decode(
    FILE * words);

void zerg1Encode(
    char **lines,
    FILE * packet);

int zerg2Encode(
    char **lines);

void zerg3Encode(
    char **lines,
    FILE * packet);

int getValue(
    char *);

double getDValue(
    char *string);

float getFValue(
    char *string);

char *extract(
    char *line);

char *
read_file(
    int filesize,
    FILE * words);

int
line_count(
    char *contents);

int
packet_count(
    char *contents);

char **
initialize(
    int *packetcount,
    const char *filename);

char **
setup(
    int *linecount,
    char *packet);

void
array_free(
    char **content_array,
    int wordcount);

