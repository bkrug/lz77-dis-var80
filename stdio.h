extern fopen(),fclose(),getc(),putc(),fgets(),fputs();
extern fread(),fwrite(),fseek(),rewind(),fdelete();
extern ferrc(),feof();
/*
** STDIO : standard c99 I/O definitions
*/
#define FILE     int
#define stdin    -1
#define stdout   -2
#define stderr   -3
#define EOF      -1
#define ERR      -2
#define YES       1
#define NO        0
#define NULL      0
#define EOL      10
#define FF       12
#define BS        8