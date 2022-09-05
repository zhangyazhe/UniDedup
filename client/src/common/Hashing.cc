#include "Group.hh"
#include "Fingerprint.hh"

pthread_t hash_t;

static void* sha1_thread(void *arg) {
    while (1) {
        struct chunk *c = sync_queue_pop(chunk_queue);

        if (c == NULL) {
            sync_queue_term(hash_queue);
            break;
        }
        sha1(c->data, c->size, c->fp);
        sync_queue_push(hash_queue, c);
    }
    return NULL;
}

void start_hash_phase() {
    hash_queue = sync_queue_new(100);
    pthread_create(&hash_t, NULL, sha1_thread, NULL);
}

void stop_hash_phase() {
    pthread_join(hash_t, NULL);
}