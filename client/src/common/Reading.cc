#include "Chunking.hh"
#include "Reading.hh"

static pthread_t read_t;

static void read_file(int fd) {
    static unsigned char buf[DEFAULT_BLOCK_SIZE];
    int size = 0;

    struct chunk *c = NULL;

    while ((size = read(fd, buf, DEFAULT_BLOCK_SIZE)) != 0) {
        c = new_chunk(size);
        memcpy(c->data, buf, size);
        sync_queue_push(read_queue, c);
    }
}

static void *read_thread(void *argv) {
    int fd = *(int*)argv;
    read_file(fd);
    sync_queue_term(read_queue);
    return NULL;
}

void start_read_phase(int fd) {
    read_queue = sync_queue_new(10);
    pthread_create(&read_t, NULL, read_thread, (void*)&fd);
}

void stop_read_phase() {
    pthread_join(read_t, NULL);
}