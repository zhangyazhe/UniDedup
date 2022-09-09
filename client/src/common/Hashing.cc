#include "Hashing.hh"

pthread_t hash_t;

static void* sha1_thread(void *arg) {
    int total = 0;
    while (1) {
        // cout << "[debug]: 3223" << endl;
        struct chunk *c = (struct chunk *) sync_queue_pop(chunk_queue);
        // cout << "[debug]: 3224" << endl;
        if (c == NULL) {
            sync_queue_term(hash_queue);
            break;
        }
        total += c->size;
        assert(c->data != NULL);
        // cout << "[debug]: 3225" << endl;
        sha1(c->data, c->size, c->fp);
        // cout << "[debug]: 3226" << endl;
        sync_queue_push(hash_queue, c);
    }
    printf("hash total size: %d\n", total);
    return NULL;
}

void start_hash_phase() {
    // cout << "[debug]: 3221" << endl;
    hash_queue = sync_queue_new(100);
    // cout << "[debug]: 3222" << endl;
    pthread_create(&hash_t, NULL, sha1_thread, NULL);
}

void stop_hash_phase() {
    pthread_join(hash_t, NULL);
}