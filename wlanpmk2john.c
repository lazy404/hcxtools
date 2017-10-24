#define _GNU_SOURCE
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#ifdef __APPLE__
#define strdupa strdup
#include <libgen.h>
#else
#include <stdio_ext.h>
#endif
#include "common.c"

/*===========================================================================*/
size_t chop(char *buffer, size_t len)
{
char *ptr = buffer +len -1;

while(len)
	{
	if (*ptr != '\n')
		break;
	*ptr-- = 0;
	len--;
	}

while(len)
	{
	if (*ptr != '\r')
		break;
	*ptr-- = 0;
	len--;
	}
return len;
}
/*---------------------------------------------------------------------------*/
int fgetline(FILE *inputstream, size_t size, char *buffer)
{
if(feof(inputstream))
	return -1;
char *buffptr = fgets (buffer, size, inputstream);

if(buffptr == NULL)
	return -1;

size_t len = strlen(buffptr);
len = chop(buffptr, len);
return len;
}

/*===========================================================================*/
void outputhashlist(FILE *fhcombi, FILE *fhhash)
{
int c;
int combilen;
int essidlen;
long int hashcount = 0;
long int skippedcount = 0;
char *essidname = NULL;
char combiline[100];

while((combilen = fgetline(fhcombi, 100, combiline)) != -1)
	{
	if(combilen < 66)
		{
		skippedcount++;
		continue;
		}
	if(combiline[64] != ':')
		{
		skippedcount++;
		continue;
		}
	essidname = strchr(combiline, ':') +1;
	if(essidname == NULL)
		{
		skippedcount++;
		continue;
		}
	essidlen = strlen(essidname);
	if((essidlen < 1) || (essidlen > 32))
		{
		skippedcount++;
		continue;
		}

	combiline[64] = 0;
	fprintf(fhhash, "$pbkdf2-hmac-sha1$4096$");
	for(c = 0; c < essidlen; c++)
		fprintf(fhhash, "%02x", essidname[c]);
	fprintf(fhhash, "$%s\n\n", combiline);
	hashcount++;
	}
printf("\r%ld hashrecords generated, %ld password(s) skipped\n", hashcount, skippedcount);
return;
}
/*===========================================================================*/
void outputsinglehash(char *pmkname, char *essidname, int essidlen)
{
int c;
printf("\nuse -form:pbkdf2-hmac-sha1 to get password\n");
printf("$pbkdf2-hmac-sha1$4096$");
for(c = 0; c < essidlen; c++)
	printf("%02x", essidname[c]);
printf("$%s\n\n", pmkname);
return;
}
/*===========================================================================*/
__attribute__ ((noreturn))
static void usage(char *eigenname)
{
printf("%s %s (C) %s ZeroBeat\n"
	"usage: %s <options>\n"
	"\n"
	"options:\n"
	"-i <file>  : input combilist (pmk:essid)\n"
 	"-o <file>  : output hashfile (-form:pbkdf2-hmac-sha1)\n"
	"-e <essid> : input single essid (networkname: 1 .. 32 characters)\n"
	"-p <pmk>   : input plainmasterkey (64 xdigits)\n"
	"-h         : this help\n"
	"\n", eigenname, VERSION, VERSION_JAHR, eigenname);
exit(EXIT_FAILURE);
}
/*===========================================================================*/
int main(int argc, char *argv[])
{
int auswahl;
int p;
int essidlen = 0;
int pmklen = 0;
char *eigenname;
char *eigenpfadname;
char *pmkname = NULL;
char *essidname = NULL;

FILE *fhcombi = NULL;
FILE *fhhash = NULL;

eigenpfadname = strdupa(argv[0]);
eigenname = basename(eigenpfadname);

setbuf(stdout, NULL);
while ((auswahl = getopt(argc, argv, "i:o:e:p:h")) != -1)
	{
	switch (auswahl)
		{
		case 'i':
		if((fhcombi = fopen(optarg, "r")) == NULL)
			{
			fprintf(stderr, "error opening %s\n", optarg);
			exit(EXIT_FAILURE);
			}
		break;

		case 'o':
		if((fhhash = fopen(optarg, "a")) == NULL)
			{
			fprintf(stderr, "error opening %s\n", optarg);
			exit(EXIT_FAILURE);
			}
		break;

		case 'e':
		essidname = optarg;
		essidlen = strlen(essidname);
		if((essidlen < 1) || (essidlen > 32))
			{
			fprintf(stderr, "error wrong essid len (allowed: 1 .. 32 characters)\n");
			exit(EXIT_FAILURE);
			}
		break;

		case 'p':
		pmkname = optarg;
		pmklen = strlen(pmkname);
		if(pmklen != 64)
			{
			fprintf(stderr, "error wrong plainmasterkey len (allowed: 64 xdigits)\n");
			exit(EXIT_FAILURE);
			}
		for(p = 0; p < 64; p++)
			{
			if(!(isxdigit(pmkname[p])))
				{
				fprintf(stderr, "error wrong plainmasterkey len (allowed: 64 xdigits)\n");
				exit(EXIT_FAILURE);
				}
			}
 		break;

		default:
		usage(eigenname);
		}
	}

if((essidname != NULL) && (pmkname != NULL))
	outputsinglehash(pmkname, essidname, essidlen);

else if((fhcombi != NULL) && (fhhash != NULL))
	outputhashlist(fhcombi, fhhash);

if(fhcombi != NULL)
	fclose(fhcombi);

if(fhhash != NULL)
	fclose(fhhash);


return EXIT_SUCCESS;
}