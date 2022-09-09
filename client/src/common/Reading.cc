#include "Reading.hh"

static pthread_t read_t;

static void read_file(const char* filepath) {
    static unsigned char buf[DEFAULT_BLOCK_SIZE];
    int size = 0;

    int fd = open(filepath, O_RDONLY);
    if(fd < 0) {
        cout << "open file " + string(filepath) +"error" << endl;
        return;
    }

    struct chunk *c = NULL;

    while ((size = read(fd, buf, DEFAULT_BLOCK_SIZE)) != 0) {
        if(size == -1) {
            cout << "[error] " + to_string(errno) + ": read file error" << endl;
            break;
        }
        // total+=size;
        c = new_chunk(size);
        if(c == NULL) {
            cout << "chunk malloc fails" << endl;
            break;
        }
        memcpy(c->data, buf, size);
        sync_queue_push(read_queue, c);
    }
    // printf("read total size: %d\n", total);
}

static void *read_thread(void *argv) {
    const char* filepath = (const char*)argv;
    read_file(filepath);
    sync_queue_term(read_queue);
    return NULL;
}

void start_read_phase(const char* buf) {
    read_queue = sync_queue_new(100);
    pthread_create(&read_t, NULL, read_thread, (void*)buf);
}

void stop_read_phase() {
    pthread_join(read_t, NULL);
}