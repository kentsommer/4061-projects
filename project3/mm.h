#ifndef __MM_H
#define __MM_H

#include <sys/time.h>

#define INTERVAL 0
#define CHUNK_SIZE 64
#define NUM_CHUNKS 1000000


/* TODO - Fill this in */
typedef struct {
	int index; // that is the first free chunk we can use
	void* data;
	int sizeOfChunk;
	int numberOfChunks;
    
} mm_t;

/* TODO - Implement these in mm.c */
double comp_time(struct timeval time_s, struct timeval time_e);
int mm_init(mm_t *mm, int num_chunks, int chunk_size);
void *mm_get(mm_t *mm);
void mm_put(mm_t *mm, void *chunk);
void mm_release(mm_t *mm);

#endif