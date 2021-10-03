#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1
#define EOFILE -1

/* ratio vs. speed constant */
/* the larger this constant, the better the compression */
#define MAXCOMPARES 75

/* unused entry flag */
#define NIL       0xFFFF

/* bits per symbol- normally 8 for general purpose compression */
#define CHARBITS  8

/* minimum match length & maximum match length */
#define THRESHOLD 2
#define MATCHBITS 4
#define MAXMATCH  ((1 << MATCHBITS) + THRESHOLD - 1)

/* sliding dictionary size and hash table's size */
/* some combinations of HASHBITS and THRESHOLD values will not work
     correctly because of the way this program hashes strings */
#define DICTBITS  12
#define HASHBITS  10
#define DICTSIZE  4096 /*(1 << DICTBITS)*/
#define HASHSIZE  1024 /*(1 << HASHBITS)*/

/* # bits to shift after each XOR hash */
/* this constant must be high enough so that only THRESHOLD + 1
     characters are in the hash accumulator at one time */
#define SHIFTBITS 4 /*((HASHBITS + THRESHOLD) / (THRESHOLD + 1))*/

/* sector size constants */
#define SCTRBIT 10
#define SCTRLEN (1 << SCTRBIT)
#define SCTRAND ((0xFFFF << SCTRBIT) & 0xFFFF)

/* stores input from a file */
#define RECSIZE 128  /* This should devide evenly in SCTRLEN*/