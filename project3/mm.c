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


int mm_init(mm_t *mm, int hm, int sz) 
{
  int i, *status;
  cs = sz;

  if((mm->data = malloc((hm + 1) * sizeof(int))) == NULL)
  {
    perror("Failed on malloc in init");
    return -1;
  }
  if((mm->status = malloc((hm + 1) * sizeof(int))) == NULL)
  {
    perror("Failed on malloc in init");
    return -1;
  }
  status = mm->status;
  for(i = 0; i < hm; i++)
  {
    *status = FREE;
    status++;
  }
  *status = END;
  mm->position = 0;
  return 0;
}

void *mm_get(mm_t *mm)
{
  void *chunk;
  int position, *status;
  status = mm->status + mm->position;
  switch(*status)
  {
    case FREE:
      chunk = mm->data + (mm->position * cs);
      *status = TAKEN;
      mm->position++;
      break;
    case TAKEN:
      position = 1;
      status = mm->status;
      while(*status == TAKEN)
      {
        status++;
        position++;
      }
      if(*status == END)
      {
        return NULL;
      }
      else
      {
        chunk = mm->data + (position * cs);
        *status = TAKEN;
        mm->position = position;
      }
      break;
    case END:
      return NULL;
    default:
      perror("Failed trying to match status");
  }
  return chunk;
}

void mm_put(mm_t *mm, void *chunk) 
{
  int index, *status;
  if(cs > 0)
  {
    index = (((char *) chunk) - ((char *) mm->data)) / cs;
  }
  else
  {
    perror("error, invalid chunky size");
  }
  if(index < mm->position)
  {
    mm->position = index;
  }
  status = mm->status + index;
  *status = FREE;
}

void mm_release(mm_t *mm)
{
  if(mm == NULL)
  {
    perror("Failed. Can't free as pointer is NULL");
  }
  free(mm->data);
}

// void mm_release(mm_t *mm)
// {
//   if(mm == NULL)
//   {
//     perror("Failed. Can't free as pointer is NULL");
//   }
//   free(mm);
// }

/*
 * TODO - This is just an example of how to use the timer.  Notice that
 * this function is not included in mm_public.h, and it is defined as static,
 * so you cannot call it from other files.  Instead, just follow this model
 * and implement your own timing code where you need it.
 */
// static void timer_example() {
//   struct timeval time_s, time_e;
//   /* start timer */
//   gettimeofday (&time_s, NULL);
//   mm_t *mm = NULL;
//   mm_init(&mm,1000000,64);
//   int i;
//   for(i=0;i<1000000;i++)
//   {
//         mm_put(&mm,&mm);
//         mm_get(&mm);
//   }

//   /* TODO - code you wish to time goes here */
//   gettimeofday(&time_e, NULL);

//   fprintf(stderr, "Time taken = %f msec\n",
//   comp_time(time_s, time_e) / 1000.0);
// }

// int main( int argc, const char* argv[] )
// {
//   timer_example();
//   return 0;
// }
