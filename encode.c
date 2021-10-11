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
char even1;

/* hashtable & link list table */
/*unsigned*/ int
    hash[HASHSIZE],
    nextlink[DICTSIZE];

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
    bitsin;

int infile, outfile;

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

/* writes multiple bit codes to the output stream */
SendBits(/*unsigned int*/ bits, /*unsigned int*/ numbits)
    int bits;
    int numbits;
{
    char encodebuf[2];
    if (bitsin == 0) {
        bitbuf = bitbuf | bits;
    }
    else {
        bitbuf = bitbuf | (bits << bitsin);
    }

    bitsin = bitsin + numbits;

    /* special case when # bits in buffer exceeds 16 */    
    if (bitsin > 16)
    {
        ++bytesput;
        encodebuf[0] = bitbuf & 255;
        if (fwrite(encodebuf, 1, outfile) == EOFILE )
        {
            puts("\nerror writing to output file");
            /*exit(EXIT_FAILURE);*/
            return EXIT_FAILURE;
        }
        if ((8 - (bitsin - numbits)) != 0) {
            bitbuf = bits >> (8 - (bitsin - numbits));
        }
        bitsin = bitsin - 8;
    }

    while (bitsin >= 8)
    {
        ++bytesput;
        encodebuf[0] = bitbuf & 255;
        if (fwrite(encodebuf, 1, outfile) == EOFILE )
        {
            puts("\nerror writing to output file");
            /*exit(EXIT_FAILURE);*/
            return EXIT_FAILURE;
        }
        /*puts("\nbitbuf");
        puts(itod(bitbuf, "       ", 7));*/
        bitbuf = bitbuf >> 8;
        bitsin = bitsin - 8;
    }
}

/* sends a match to the output stream */
SendMatch(/*unsigned int */ matchlen, /*unsigned int*/ matchdistance)
    int matchlen;
    int matchdistance;
{
    SendBits(1, 1);

    SendBits(matchlen - (THRESHOLD + 1), MATCHBITS);

    SendBits(matchdistance, DICTBITS);

    /*puts(" match ");
    puts(itod(matchdistance, "      ", 6));
    puts(itod(matchlen, "      ", 6));*/
}

/* sends one character (or literal) to the output stream */
SendChar(/*unsigned int*/ character)
    int character;
{
    char shortCharacter[2];

    SendBits(0, 1);

    SendBits(character, CHARBITS);

    /*if (character > 31 && character < 127) {
        shortCharacter[0] = character;
        shortCharacter[1] = 0;
        puts(shortCharacter);
    }
    else {
        puts(itod(character, "      ", 5));
    }*/
}

/* initializes the search structures needed for compression */
InitEncode()
{
    /*register unsigned*/ int i;

    for (i = 0; i < HASHSIZE; i++) hash[i] = NIL;

    nextlink[DICTSIZE] = NIL;
}

/* Read enough records to fill an algorithm sector */
ReadDV80(startingpos)
    int startingpos;
{
    int c, alreadyread, dictpos;
    char tempStr[2];
    alreadyread = 0;
    dictpos = startingpos;
    if (feof(infile))
        return EOFILE;
    if (eofinfile == 1)
        return EOFILE;
    while (alreadyread < SCTRLEN)
    {
        c = fread(buff, 80, infile);
        if (c < 0) {
            eofinfile = 1;
            return alreadyread;
        }
        alreadyread = alreadyread + 1;
        dict[dictpos++] = c & 0xFF;
        for(buffpos = 0; buffpos < c; ++buffpos)
        {
            alreadyread = alreadyread + 1;
            dict[dictpos++] = buff[buffpos];
        }
        if (feof(infile)) {
            eofinfile = 1;
            return alreadyread;
        }
    }
    return alreadyread;
}

/* loads dictionary with characters from the input stream */
/*unsigned int*/ LoadDict(/*unsigned int */ dictpos)
    int dictpos;
{
    /*register unsigned*/ int i, j;
    /*puts("\nLoading Data");*/

    /*sizeof (char) == 1*/
    if ((i = ReadDV80(dictpos)) == EOFILE )
    {
        return 0;
        /*puts("\nerror reading from input file");*/
        /*exit(EXIT_FAILURE);*/
        /*return EXIT_FAILURE;*/
    }

    /* since the dictionary is a ring buffer, copy the characters at
         the very start of the dictionary to the end */
    if (dictpos == 0)
    {
        for (j = 0; j < MAXMATCH; j++)
        {
            dict[j + DICTSIZE] = dict[j];
        }
    }

    return i;
}

/* deletes data from the dictionary search structures */
/* this is only done when the number of bytes to be
     compressed exceeds the dictionary's size */
DeleteData(/*unsigned int*/ dictpos)
    int dictpos;
{

    /*register unsigned*/ int i, j;

    j = dictpos;    /* put dictpos in register for more speed */

    /* delete all references to the sector being deleted */
    /*for (i = 0; i < DICTSIZE; i++)
        if ((nextlink[i] & SCTRAND) == j) nextlink[i] = NIL;
    for (i = 0; i < HASHSIZE; i++)
        if ((hash[i] & SCTRAND) == j) hash[i] = NIL;*/

    /* delete everything. The sector size is not constant */
    for (i = 0; i < DICTSIZE; i++)
        nextlink[i] = NIL;
    for (i = 0; i < HASHSIZE; i++)
        hash[i] = NIL;
}

/* hash data just entered into dictionary */
/* XOR hashing is used here, but practically any hash function will work */
HashData(/*unsigned int */dictpos, /*unsigned int */bytestodo)
    int dictpos;
    int bytestodo;
{
    /*register unsigned*/ int i, j, k, chrAtD;
    /*puts("\nHashing Data");*/

    if (bytestodo <= THRESHOLD)
    {
        /* not enough bytes in sector for match? */
        for (i = 0; i < bytestodo; i++)
        {
            nextlink[dictpos + i] = NIL;
        }
    }
    else
    {
        /* matches can't cross sector boundries */
        for (i = bytestodo - THRESHOLD; i < bytestodo; i++) {
            nextlink[dictpos + i] = NIL;
        }

        /*unsigned*/ 
        chrAtD = dict[dictpos];
        j = (chrAtD << SHIFTBITS) ^ dict[dictpos + 1];

        /* calculate end of sector */
        k = dictpos + bytestodo - THRESHOLD;

        for (i = dictpos; i < k; i++)
        {
            nextlink[i] = hash[
                j = (((j << SHIFTBITS) & (HASHSIZE - 1)) 
                    ^ dict[i + THRESHOLD])
            ];
            hash[j] = i;
        }
    }

    /*puts("\n");
    for (i = 0; i < 50; ++i) {
        puts(itod(nextlink[i], "      ", 6));
    }*/
}

/* finds match for string at position dictpos */
/* this search code finds the longest AND closest
     match for the string at dictpos */
/*FindMatch(unsigned int dictpos, unsigned int startlen)*/
FindMatch(dctpos, startlen)
    int dctpos;
    int startlen;
{
    /*register unsigned*/ int i, j, k;
    /* Declaring make_even seems to fix the same type of bug that 
       assembly's EVEN command fixes.*/
    /*unsigned*/ char l, make_even;
    int retVal[2];

    i = dctpos;
    matchlength = startlen;
    k = MAXCOMPARES;
    l = dict[dctpos + matchlength];

    do
    {
        if ((i = nextlink[i]) == NIL) {
            retVal[0] = matchpos;
            retVal[1] = matchlength;
            return retVal;
            /* get next string in list */
        }

        if (dict[i + matchlength] == l)
        /* possible larger match? */
        {
            for (j = 0; j < MAXMATCH; j++)      
                /* compare strings */
                if (dict[dctpos + j] != dict[i + j])
                    break;

            if (j > matchlength)  /* found larger match? */
            {
                matchlength = j;
                matchpos = i;
                if (matchlength == MAXMATCH) {
                    retVal[0] = matchpos;
                    retVal[1] = matchlength;
                    return retVal;
                }
                /* exit if largest possible match */
                l = dict[dctpos + matchlength];
            }
        }
    }
    while (--k);  /* keep on trying until we run out of chances */
}

/* finds dictionary matches for characters in current sector */
/*DictSearch(unsigned int dictpos, unsigned int bytestodo)*/
DictSearch(dictps, bytestodo)
    int dictps;
    int bytestodo;
{
    /*register unsigned*/ int i, j;
    /*unsigned*/ int oldlen, oldpos;
    int* foundMatchVals;
    /*puts("\nSearching Dictionary");*/

    /* non-greedy search loop (slow) */

    i = dictps;
    j = bytestodo;

    while (j)
    /* loop while there are still characters left to be compressed */
    {
        /*matchlength is a global variable, but somehow
         it is getting corrupted after returning from FindMatch */
        foundMatchVals = FindMatch(i, THRESHOLD);
        matchpos = foundMatchVals[0];
        matchlength = foundMatchVals[1];

        if (matchlength > THRESHOLD)
        {
            oldlen = matchlength;
            oldpos = matchpos;

            for ( ; ; )
            {
                foundMatchVals = FindMatch(i + 1, oldlen);
                matchpos = foundMatchVals[0];
                matchlength = foundMatchVals[1];

                if (matchlength > oldlen)
                {
                    oldlen = matchlength;
                    oldpos = matchpos;
                    SendChar(dict[i++]);
                    j--;
                }
                else
                {
                    if (oldlen > j)
                    {
                        oldlen = j;
                        if (oldlen <= THRESHOLD)
                        {
                            SendChar(dict[i++]);
                            j--;
                            break;
                        }
                    }

                    SendMatch(oldlen, (i - oldpos) & (DICTSIZE - 1));
                    i = i + oldlen;
                    j = j - oldlen;
                    break;
                }
            }
        }
        else
        {
            SendChar(dict[i++]);
            j--;
        }
    }
}

/* main encoder */
Encode ()
{
    /*unsigned*/ int dictpos, deleteflag, sectorlen;
    /*unsigned long*/ int bytescompressed;
    int hashIndex;

    InitEncode();
    puts("\nInitialized");

    matchlength = matchpos = 0;
    bitbuf = bitsin = bytesput = 0;
    buffpos = dictpos = deleteflag = 0;
    eofinfile = 0;
    bytescompressed = 0;

    while (1)
    {
        /* delete old data from dictionary */
        if (deleteflag) DeleteData(dictpos);

        /* grab more data to compress */
        if ((sectorlen = LoadDict(dictpos)) == 0) break;
 
        /* hash the data */
        HashData(dictpos, sectorlen);

        puts("\nSectorLen");
        puts(itod(sectorlen, "      ", 7));
        /* find dictionary matches */
        DictSearch(dictpos, sectorlen);

        bytescompressed = bytescompressed + sectorlen;

        /*puts("\r%ld", bytescompressed);*/

        dictpos = dictpos + sectorlen;

        /* wrap back to beginning of dictionary when its full */
        if (dictpos >= DICTSIZE - SCTRLEN - 81)
        {
            dictpos = 0;
            deleteflag = 1;   /* ok to delete now */
        }
    }

    /* Send EOF flag */
    SendMatch(MAXMATCH + 1, 0);

    /* Flush bit buffer */
    if (bitsin) SendBits(0, 8 - bitsin);

    return;
}

main()
{
    infile = fopen("DSK3.CATCAR.TXT", "r80");
    outfile = fopen("DSK3.CATCAR.77", "W1");
    if (infile == NIL)
    {
        /*puts("??? %s\n", infile);*/
        puts("could not open input file");
        return EXIT_FAILURE;
    }
    if (outfile == NIL)
    {
        /*puts("??? %s\n", infile);*/
        puts("could not open output file");
        return EXIT_FAILURE;
    }
    puts("file opened");
    Encode();
    puts("\nencoding complete");
    fclose(infile);
    fclose(outfile);
    puts("\nfile closed");

    return EXIT_SUCCESS;
}