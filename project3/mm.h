/*CSci4061 F2014 Assignment 3
*section: 4
*date: 11/10/14
*names: Kent Sommer, Kanad Gupta, Xi Chen
*id: somme282, kgupta, chen2806
*/

#ifndef __MM_H
#define __MM_H

#include <sys/time.h>

#define INTERVAL 0
#define CHUNK_SIZE 64
#define NUM_CHUNKS 1000000
#define FREE 0
#define TAKEN 1
#define END 2

static int cs; 

/* TODO - Fill this in */
typedef struct {
	void* data;
    int *status; //FREE = 0, TAKEN = 1, END = 2;
    int position; 
} mm_t;

/* TODO - Implement these in mm.c */
double comp_time(struct timeval time_s, struct timeval time_e);
int mm_init(mm_t *mm, int num_chunks, int chunk_size);
void *mm_get(mm_t *mm);
void mm_put(mm_t *mm, void *chunk);
void mm_release(mm_t *mm);

#endif