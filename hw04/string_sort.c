#include "stdio.h"
#include <stdlib.h>
#include <string.h>

int compStrings(const void* str1, const void* str2)
{
	/** jeez, this is confusing
	** function gets a void*, which is a VOID POINTER to a char*
	** so we're not looking at char*, we're looking at POINTERS to char*
	** ie str1 is a void pointer to a char*
	** we don't want void pointers to char*
	** we want, at the very least, CHAR pointers to char*
	** so, we do that first.   (char**)str1
	** but strcmp doesn't want char**, it wants normal strings,
	** char*.  So, we dereference again.  *(char**)
	** that seems to be the consensus on how this casting stuff is supposed
	** to work.  99% sure I got it right.
	*/
	return strcmp(*(char**)str1, *(char**)str2);
}

int main(int argc, char** argv)
{
	int strNum = atoi(argv[1]); // read the number of strings we are taking in and convert it to an int
	int strLen = 80; // max string length, as per hints
	char** strArr = malloc(strNum * sizeof(char*)); // make an array of pointers to char (ie strings)
	int i = 0; // loop counter (the way I have gcc set up, it complains if I stick this inside, so whatever)
	for(i = 0; i < strNum; i++)
	{
		strArr[i] = malloc(strLen * sizeof(char)); // allocate space for each individual string
	}

	// read in all the strings
	for(i = 0; i < strNum; i++)
	{
		fgets(strArr[i], strLen, stdin);
	}

	qsort(strArr, strNum, sizeof(char*), compStrings); // do the actual sort

	for(i = 0; i < strNum; i++)
	{
		printf("%s", strArr[i]); // let's see all the strings we read!
	}

	for(i = 0; i < strNum; i++)
	{
		free(strArr[i]); // free the memory used by each individual array entry...
	}
	free(strArr); // ...and the array itself

	return 0;
}
