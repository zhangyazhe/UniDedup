#ifndef _CHUNKING_HH_
#define _CHUNKING_HH_

#include "Fingerprint.hh"

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

int rabin_chunk_data(unsigned char *p, int n);
int normalized_rabin_chunk_data(unsigned char *p, int n);
int tttd_chunk_data(unsigned char *p, int n);
static inline int fixed_chunk_data(unsigned char *buf, int size);
int ae_chunk_data(unsigned char *p, int n);

#endif