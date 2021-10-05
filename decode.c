/* Taken from https://gist.github.com/fogus/5401265 and       */
/*     modified for C99                       */
/* PROG1.C                            */
/* Simple Hashing LZ77 Sliding Dictionary Compression Program */
/* By Rich Geldreich, Jr. October, 1993               */
/* Originally compiled with QuickC v2.5 in the small model.   */
#include stdio.h
#include lz77.h

/* dictionary plus MAXMATCH extra chars for string comparisions */
/*unsigned char
    dict[DICTSIZE + MAXMATCH];*/
char dict[4113];

/* stores input from a file */
char buff[RECSIZE];
int buffpos;
char eofinfile;
char bytesput;

/* misc. global variables */
/*unsigned*/ int
    matchlength,
    matchpos,
    bitbuf,
    bitsin,
    masks[17] = {
        0,1,3,7,15,31,63,127,255,
        511,1023,2047,4095,8191,
        16383,32767,65535
    };

int infile, outfile;

/* reads multiple bit codes from the input stream */
/*unsigned int*/ReadBits(/*unsigned int*/numbits)
    int numbits;
{

    /*register unsigned*/ int i;
    char encodebuf[1];
    char tempStr[2];

    if (8 - bitsin == 0) {
        i = bitbuf;
    }
    else {
        i = bitbuf >> (8 - bitsin);
    }

    while (numbits > bitsin)
    {
        fread(encodebuf, 1, infile);
        if ((bitbuf = encodebuf[0]) == EOFILE)
        {
            puts("\nerror reading from input file");
            /*exit(EXIT_FAILURE);*/
            return EXIT_FAILURE;
        }
        if (bitsin == 0) {
            i = i | bitbuf;
        }
        else {
            i = i | (bitbuf << bitsin);
        }
        bitsin = bitsin + 8;
    }

    bitsin = bitsin - numbits;

    return (i & masks[numbits]);
}

WriteLine(startpos, maxpos)
    int startpos;
    int maxpos;
{
    int b, c, curpos;
    /*char *lineToPrint;
    lineToPrint = dict;
    lineToPrint = lineToPrint + startpos;*/
    for (b = 0, c = startpos; c <= maxpos && b < 128; ) {
        buff[b++] = dict[c++];
    }
    puts(buff);
    c = fputs(buff, outfile);
    /*if (c == EOFILE) {
        puts("\nerror writing to output file");
        for (c = 0; c < 30000; ++c) {}
        exit(EXIT_FAILURE);
    }*/
        
    /*curpos = startpos;
    {
    } while (curpos < maxpos && dict[curpos++] != 0)
    return curpos;*/
    return maxpos;
}

/* main decoder */
Decode ()
{
    /*register unsigned*/ int i, j, k;
    /*unsigned long*/ int bytesdecompressed, lineStart;
    char displayStr[2];

    bitbuf = bitsin = 0;
    i = 0;
    lineStart = bytesdecompressed = 0;

    for ( ; ; )
    {
        if (ReadBits(1) == 0)   /* character or match? */
        {
            dict[i++] = ReadBits(CHARBITS);
            /*displayStr[0] = dict[i-1];
            displayStr[1] = 0;
            puts(displayStr);*/
            if (dict[i-1] == 0)
            {
                lineStart = WriteLine(lineStart, i);
            }
            if (i == DICTSIZE)
            {
                i = 0;
                bytesdecompressed = bytesdecompressed + DICTSIZE;
            }            
        }
        else
        {
            /* get match length from input stream */
            k = (THRESHOLD + 1) + ReadBits(MATCHBITS);
            /*puts("{{");
            puts(itod(k, "      ", 6));
            puts("}}");*/

            if (k == (MAXMATCH + 1))
            {
                /* Found End of Source Data */
                if (lineStart < i) {
                    WriteLine(lineStart, i);
                }
                bytesdecompressed = bytesdecompressed + i;
                return;
            }

            /* get match position from input stream */
            j = ((i - ReadBits(DICTBITS)) & (DICTSIZE - 1));
            /*puts("[[");
            puts(itod(j, "      ", 6));
            puts("]]");*/

            if ((i + k) >= DICTSIZE)
            {
                do
                {
                    dict[i++] = dict[j++];
                    /*displayStr[0] = dict[i-1];
                    displayStr[1] = 0;
                    puts(displayStr);*/
                    j = j & (DICTSIZE - 1);
                    if (dict[i-1] == 0)
                    {
                        lineStart = WriteLine(lineStart, i);
                    }
                    if (i == DICTSIZE)
                    {
                        bytesdecompressed = bytesdecompressed + DICTSIZE;
                        i = 0;
                    }
                }
                while (--k);
            }
            else
            {
                if ((j + k) >= DICTSIZE)
                {
                    do
                    {
                        dict[i++] = dict[j++];
                        /*displayStr[0] = dict[i-1];
                        displayStr[1] = 0;
                        puts(displayStr);*/
                        j = j & (DICTSIZE - 1);
                        if (dict[i-1] == 0)
                        {
                            lineStart = WriteLine(lineStart, i);
                        }
                    }
                    while (--k);
                }
                else
                {
                    do
                    {
                        dict[i++] = dict[j++];
                        /*displayStr[0] = dict[i-1];
                        displayStr[1] = 0;
                        puts(displayStr);*/
                        if (dict[i-1] == 0)
                        {
                            lineStart = WriteLine(lineStart, i);
                        }
                    }
                    while (--k);
                }
            }
        }
    }
}

main()
{
    infile = fopen("DSK3.CATCAR.77", "R1");
    outfile = fopen("DSK3.CATCAR2.TXT", "w80");
    if (infile == NIL)
    {
        puts("could not open input file");
        return EXIT_FAILURE;
    }
    if (outfile == NIL)
    {
        puts("could not open output file");
        return EXIT_FAILURE;
    }
    Decode();
    puts("\ndecoding complete");
    fclose(infile);
    fclose(outfile);
 
     return EXIT_SUCCESS;
}

/*
** s=itod(nbr,str,sz) -
**
** convert nbr to signed decimal string of width sz
** right justified, blank filled, result in str[].
** sz includes 0-byte string terminator. returns str.
*/
itod(nbr,str,sz) int nbr,sz; char str[];
{
  int sgn;
  sgn=' ';
  if(nbr<0) { nbr=-nbr; sgn='-'; }
  str[--sz]=0;
  while(sz)
  { str[--sz]=nbr%10+'0';
    if(!(nbr=nbr/10))break;
  }
  if(sz) str[--sz]=sgn;
  while(sz) str[--sz]=' ';
  return str;
}