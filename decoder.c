struct FileHeader
{
    int FileType: 32;
    int MajorVer: 16;
    int MinorVer: 16;
    int GMT:    : 32;
    int Acc:    : 32;
    int MaxLen  : 32;
};

struct PcapHeader
{
    int Epoch   : 32;
    int EpochMil: 32;
    int DataLen : 32;
    int PackLen : 32;
};

struct EthernetHeader
{
    int Dmac    : 48;
    int Smac    : 48;
    int Etype   : 16;
};

struct Ipv4Header
{
    int Version : 4;
    int IHL     : 4;
    int DSCP    : 6;
    int ECN     : 2;
};

struct UdpHeader
{
    int Sport   : 16;
    int Dport   : 16;
    int Len     : 16;
    int CheckSum: 16;
};

struct Zerg
{
    int Version : 4;
    int Type    : 4;//////////
    int TotalLen: 40;
    int Sid     : 16;
    int Did     : 16;
    int Sequence: 32;
};

struct Message
{
    //Might not be needed. Entire payload is text.
    //"Not null terminated" so will have to cut off at packet len.
};

struct Status
{
    int HP     : 24;
    int Armor  : 8;
    int MaxHP  : 24;
    int Type   : 8;
    int Speed  : 32;
    char *Name : 32;
};

struct Command
{
    int Command: 16;
    int Param1 : 16;
    int Param2 : 32;
};

struct GPS
{
    int Longit : 64;
    int Latit  : 64;
    int Altit  : 32;
    int Bearing: 32;
    int Speed  : 32;
    int Acc    : 32;
};

/*Read in file.*/
char * read_file(int filesize, FILE *words)
{
    char *contents = malloc(filesize);
    fread(contents, sizeof(char), filesize, words);
    fclose(words);
    return(contents);
}

/*Get size of file*/
int file_size(FILE *words)
{

    long start = ftell(words);
    fseek(words, 0, SEEK_END);
    long end = ftell(words);
    int filesize = (end - start) + 1;
    printf("\nfilesize: %i\n", filesize);
    fseek(words, 0, SEEK_SET);
    return(filesize);
}

int main(void)
{
    FILE *words = fopen("hello.pcap", "r");
    int filesize = file_size(words);
    char *contents = read_file(filesize, words);
    struct FileHeader fh;
    
}