// Read the data from read queue, add chunk to chunk queue after chunking
#ifndef _CHUNKING_HH_
#define _CHUNKING_HH_

#include "Fingerprint.hh"
#include "Group.hh"
#include <string.h>
#include "../util/sync_queue.hh"
#include "Config.hh"

#define CHUNK_FIXED 0
#define CHUNK_RABIN 1
#define CHUNK_NORMALIZED_RABIN 2
#define CHUNK_FILE 3 /* approximate file-level */
#define CHUNK_AE 4 /* Asymmetric Extremum CDC */
#define CHUNK_TTTD 5


struct chunk {
    fingerprint fp;
    uint32_t size;
    unsigned char* data;
};
/* Recode config information about chunking phase */
struct chunk_meta_data{
    int chunk_algorithm;
    int chunk_max_size;
    int chunk_min_size;
    int chunk_avg_size;
};

extern struct chunk_meta_data chunkMetaData;

void windows_reset();
void chunkAlg_init();
int rabin_chunk_data(unsigned char *p, int n);
int normalized_rabin_chunk_data(unsigned char *p, int n);

void ae_init();
int ae_chunk_data(unsigned char *p, int n);

int tttd_chunk_data(unsigned char *p, int n);
static inline int fixed_chunk_data(unsigned char *buf, int size);

struct chunk* new_chunk(uint32_t size);
void free_chunk(struct chunk *ck);

void start_chunk_phase();
void stop_chunk_phase();

#endif