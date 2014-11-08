#include <stdio.h>
#include <stdlib.h>

#include "mm.h"

/* Return usec */
double comp_time(struct timeval time_s, struct timeval time_e) {

  double elap = 0.0;
   
  if (time_e.tv_sec > time_s.tv_sec) {
    elap += (time_e.tv_sec - time_s.tv_sec - 1) * 1000000.0;
    elap += time_e.tv_usec + (1000000 - time_s.tv_usec);
  }
  else {
    elap = time_e.tv_usec - time_s.tv_usec;
  }
  return elap;
}

/* TODO - Implement.  Return 0 for success, or -1 and set errno on fail. */
int mm_init(mm_t *mm, int hm, int sz) {
  printf("0");
  if(hm<=0||sz<=0)
  {
    return -1;
  }
  printf("1");
  mm->sizeOfChunk = sz;
  mm->numberOfChunks = hm;
  int totalSize = hm * sz;
  printf("2");
  mm->data = (void *) malloc(sizeof(totalSize));
  mm->index=0;
  if(mm->data==NULL)
  {
    return -1;
  }
  return 0;  /* TODO - return the right value */
}

void *mm_get(mm_t *mm) {
  void* address;
  address=mm->data  + (mm->index)*(mm->sizeOfChunk);
  // may need calibrate constnat
  return address;
}

void mm_put(mm_t *mm, void *chunk) {
  if(sizeof(chunk)>(mm->sizeOfChunk))
  {
    // give an error yo
  }
  //how to handle filled memory
  void* address=mm_get(mm);
  //*address=chunk;
  mm->index++;
}

void mm_release(mm_t *mm) {
  mm->sizeOfChunk=0;
  mm->numberOfChunks=0;
  free(mm->data);
  mm->index=0;
}
/*
 * TODO - This is just an example of how to use the timer.  Notice that
 * this function is not included in mm_public.h, and it is defined as static,
 * so you cannot call it from other files.  Instead, just follow this model
 * and implement your own timing code where you need it.
 */
static void timer_example() {
  struct timeval time_s, time_e;
  /* start timer */
  gettimeofday (&time_s, NULL);
  mm_t *mm = NULL;
  mm_init(mm,100,100);
  /* TODO - code you wish to time goes here */
  gettimeofday(&time_e, NULL);

  fprintf(stderr, "Time taken = %f msec\n",
  comp_time(time_s, time_e) / 1000.0);
}

int main( int argc, const char* argv[] )
{
  timer_example();
  return 0;
}