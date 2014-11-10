/*CSci4061 F2014 Assignment 3
*section: 4
*date: 11/10/14
*names: Kent Sommer, Kanad Gupta, Xi Chen
*id: somme282, kgupta, chen2806
*/

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
  if((mm->data = malloc((hm + 1) * sizeof(int))) == NULL) // Allocate space 
  {
    perror("Failed to malloc");
    return -1;
  }
  if((mm->status = malloc((hm + 1) * sizeof(int))) == NULL) // Allocate status
  {
    perror("Failed to malloc");
    return -1;
  }
  status = mm->status;
  for(i = 0; i < hm; i++) // Set up the status for each block to FREE
  {
    *status = FREE;
    status++;
  }
  *status = END; // Set last position to END status
  mm->position = 0;
  return 0;
}

void *mm_get(mm_t *mm)
{
  void *chunk;
  int position, *status;
  status = mm->status + mm->position;
  switch(*status) // Switch through status
  {
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
    case FREE:
      chunk = mm->data + (mm->position * cs);
      *status = TAKEN;
      mm->position++;
      break;
    case END:
      return NULL;
    default:
      perror("Failed. Invalid status");
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
    perror("Failed. invalid chunk size");
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
