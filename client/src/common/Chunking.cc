#include "Chunking.hh"

static pthread_t chunk_t;

static int (*chunking)(unsigned char *buf, int size);

static inline int fixed_chunk_data(unsigned char* buf, int size){
	return chunkMetaData.chunk_avg_size > size ? size : chunkMetaData.chunk_avg_size;
}

struct chunk* new_chunk(uint32_t size) {
    struct chunk *ck = (struct chunk*) malloc(sizeof(struct chunk));
    memset(ck->fp, 0x0, sizeof(fingerprint));
    ck->size = size;

    if (size > 0) {
        ck->data = (unsigned char*) malloc(size);
    } else {
        ck->data = NULL;
    }
    return ck;
}

void free_chunk(struct chunk *ck) {
    if (ck->data) {
        free(ck->data);
        ck->data = NULL;
    }
    free(ck);
}

static void* chunk_thread(void *arg) {
    int leftlen = 0;
    int leftoff = 0;
    unsigned char *leftbuf = (unsigned char*) malloc(DEFAULT_BLOCK_SIZE + chunkMetaData.chunk_max_size);

    // unsigned char *data = malloc(chunkMetaData.chunk_max_size);

    // cout << chunkMetaData.chunk_max_size << endl;

    struct chunk *c = NULL;

    while (1) {
        c = (struct chunk*) sync_queue_pop(read_queue);

        while  ((leftlen < chunkMetaData.chunk_max_size)) {
            memmove(leftbuf, leftbuf + leftoff, leftlen);
            leftoff = 0;
            if (c != NULL) {
                assert(c->data != NULL);
                assert(c->size != 0);
                memcpy(leftbuf + leftlen, c->data, c->size);
                leftlen += c->size;
                free_chunk(c);
            } else {
                break;
            }
            c = (struct chunk*) sync_queue_pop(read_queue);
        }
        if (leftlen == 0 && c == NULL) {
            // assert(c == NULL);
            break;
        }
        // cout << "leftoff:" << leftoff << ", leftlen:" << leftlen << endl;
        int chunk_size = chunking(leftbuf + leftoff, leftlen);
        assert(chunk_size != 0);
        struct chunk *nc = new_chunk(chunk_size);
        memcpy(nc->data, leftbuf + leftoff, chunk_size);
        leftlen -= chunk_size;
        leftoff += chunk_size;

        sync_queue_push(chunk_queue, nc);
    }
    sync_queue_term(chunk_queue);
    free(leftbuf);
    return NULL;
}

void start_chunk_phase() {
    if (chunkMetaData.chunk_algorithm == CHUNK_RABIN) {
        int pwr;
        for (pwr = 0; chunkMetaData.chunk_avg_size; pwr++) {
            chunkMetaData.chunk_avg_size >>= 1;
        }
        chunkMetaData.chunk_avg_size = 1 << (pwr - 1);
        chunkAlg_init();
        chunking = rabin_chunk_data;
    } else if (chunkMetaData.chunk_algorithm == CHUNK_NORMALIZED_RABIN) {
        int pwr;
        for (pwr = 0; chunkMetaData.chunk_avg_size; pwr++) {
            chunkMetaData.chunk_avg_size >>= 1;
        }
        chunkMetaData.chunk_avg_size = 1 << (pwr - 1);
        chunkAlg_init();
        chunking = normalized_rabin_chunk_data;
    } else if (chunkMetaData.chunk_algorithm == CHUNK_TTTD) {
        int pwr;
        for (pwr = 0; chunkMetaData.chunk_avg_size; pwr++) {
            chunkMetaData.chunk_avg_size >>= 1;
        }
        chunkMetaData.chunk_avg_size = 1 << (pwr - 1);

        chunkAlg_init();
        chunking = tttd_chunk_data;
    } else if (chunkMetaData.chunk_algorithm == CHUNK_FIXED) {
        chunkMetaData.chunk_max_size = chunkMetaData.chunk_avg_size;
        chunking = fixed_chunk_data;
    } else if (chunkMetaData.chunk_algorithm == CHUNK_FILE) {
        chunkMetaData.chunk_avg_size = 4194304ll - 32768ll;
        chunkMetaData.chunk_max_size = 4194304ll - 32768ll;
        chunking = fixed_chunk_data;
    } else if (chunkMetaData.chunk_algorithm == CHUNK_AE) {
        chunking = ae_chunk_data;
        ae_init();
    } else {
        fprintf(stderr, "Invalid chunking algorithm.\n");
        exit(1);
    }

    chunk_queue = sync_queue_new(100);
    pthread_create(&chunk_t, NULL, chunk_thread, NULL);
}

void stop_chunk_phase() {
    pthread_join(chunk_t, NULL);
    printf("chunk phase stops successfully!\n");
}