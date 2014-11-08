#include "mm.h"
#include <stdio.h>
#include <stdlib.h>

struct timeval time_s, time_e;

int main()
{
	mm_t mm;

	if(mm_init(&mm, 1000000, 64) == 0)
	{
		printf("success\n");
	}
	else
	{
		printf("failure\n");
	}
  	gettimeofday (&time_s, NULL);


	mm_t *mmArray[1000000];

	int i;
	for (i = 0; i < 1000000; i++){
		if ((mmArray[i] = mm_get(&mm)) == NULL){
			printf("allocation for i = %d failed\n", i);
		}
	}

	while (i > 0){
		i--;
		mm_put(&mm, mmArray[i]);
	}


  	gettimeofday(&time_e, NULL);

	mm_release(&mm);

  	fprintf(stderr, "main_mm duration = %f msec\n", comp_time(time_s, time_e) / 1000.0);

	return 0;
}