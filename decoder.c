struct FileHeader
{
    long unsigned int FileType: 32;
    long unsigned int MajorVer: 16;
    long unsigned int MinorVer: 16;
    long unsigned int GMT     : 32;
    long unsigned int Acc     : 32;
    long unsigned int MaxLen  : 32;
};

struct PcapHeader
{
    long unsigned int Epoch   : 32;
    long unsigned int EpochMil: 32;
    long unsigned int DataLen : 32;
    long unsigned int PackLen : 32;
};

struct EthernetHeader
{
    long unsigned int Dmac    : 48;
    long unsigned int Smac    : 48;
    long unsigned int Etype   : 16;
};

struct Ipv4Header
{
    long unsigned int Version : 4;
    long unsigned int IHL     : 4;
    long unsigned int DSCP    : 6;
    long unsigned int ECN     : 2;
};

struct UdpHeader
{
    long unsigned int Sport   : 16;
    long unsigned int Dport   : 16;
    long unsigned int Len     : 16;
    long unsigned int CheckSum: 16;
};

struct Zerg
{
    long unsigned int Version : 4;
    long unsigned int Type    : 4;//////////
    long unsigned int TotalLen: 40;
    long unsigned int Sid     : 16;
    long unsigned int Did     : 16;
    long unsigned int Sequence: 32;
};

struct Message
{
    //Might not be needed. Entire payload is text.
    //"Not null terminated" so will have to cut off at packet len.
};

struct Status
{
    long unsigned int HP     : 24;
    long unsigned int Armor  : 8;
    long unsigned int MaxHP  : 24;
    long unsigned int Type   : 8;
    long unsigned int Speed  : 32;
    char *Name : 32;
};

struct Command
{
    long unsigned int Command: 16;
    long unsigned int Param1 : 16;
    long unsigned int Param2 : 32;
};

struct GPS
{
    long unsigned int Longit : 64;
    long unsigned int Latit  : 64;
    long unsigned int Altit  : 32;
    long unsigned int Bearing: 32;
    long unsigned int Speed  : 32;
    long unsigned int Acc    : 32;
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
    //int filesize = file_size(words);
    //char *contents = read_file(filesize, words);
    struct FileHeader fh;
    fread(fh.FileType, sizeof(fh.FileType), 1, words);
    printf("%x\n", fh.FileType);
    
}