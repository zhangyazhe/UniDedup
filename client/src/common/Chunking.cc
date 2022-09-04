#include "Chunking.hh"

static int (*chunking)(unsigned char *buf, int size);

struct chunk* new_chunk(uint32_t size) {
    struct chunk *ck = (struct chunk*) malloc(sizeof(struct chunk));
    memset(ck->fp, 0x0, sizeof(fingerprint));
    ck->size = size;

    if (size > 0) {
        ck->data = malloc(size);
    } else {
        ck->data = NULL;
    }
    return NULL;
}

void free_chunk(struct chunk *ck) {
    if (ck->data) {
        free(ck->data);
        ck->data = NULL;
    }
    free(ck);
}

void start_chunk_phase() {
    if (chunkMetaData.chunk_algorithm == CHUNK_RABIN) {
        int pwr;
        for (pwr = 0; chunkMetaData.chunk_avg_size; pwr++) {
            chunkMetaData.chunk_avg_size >>= 1;
        }
        chunkMetaData.chunk_avg_size = 1 << (pwr - 1);
        chunkAlg_init();
    }
}