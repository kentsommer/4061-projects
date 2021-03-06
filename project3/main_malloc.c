/*CSci4061 F2014 Assignment 3
*section: 4
*date: 11/10/14
*names: Kent Sommer, Kanad Gupta, Xi Chen
*id: somme282, kgupta, chen2806
*/

#include <stdio.h>
#include <stdlib.h>

#include "mm.h"

struct timeval time_s, time_e;

int main()
{
	gettimeofday (&time_s, NULL);
	mm_t *mmArray[1000000];
	int i;
	for (i = 0; i < 1000000; i++){
		if ((mmArray[i] = (mm_t *) malloc(64)) == NULL){
			printf("Failed to allocate i = %d\n", i);
		}
	}
	while (i > 0)
	{
		i--;
		free(mmArray[i]);
	}
  	gettimeofday(&time_e, NULL);
  	fprintf(stderr, "main_malloc duration = %f msec\n", comp_time(time_s, time_e) / 1000.0);
	return 0;
}