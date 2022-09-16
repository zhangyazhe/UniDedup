#include "Assembling.hh"

pthread_t assemble_t;

static void do_assemble(char *path) {
    printf("[debug] client, do_assemble begin.\n");
    // const char *path = (const char *)argv;
    int fd = -1;
    fd = open(path, O_WRONLY | O_CREAT, S_IRWXU);
    if (fd < 0) {
        fprintf(stderr, "assemble: open file failed\n");
        return ;
    }
    printf("[debug] client, do_assemble 0.\n");
    unsigned char assembly_area[ASSEMBLE_BUFFER_LEN] = {'0'};
    printf("[debug] client, do_assemble 1.\n");
    int offset = 0;
    struct chunk *c = NULL;
    while ((c = (struct chunk*)sync_queue_pop(receive_queue)) != nullptr) {
        assert(c->size < ASSEMBLE_BUFFER_LEN * sizeof(unsigned char));
        if (offset + c->size < ASSEMBLE_BUFFER_LEN * sizeof(unsigned char)) {
            memcpy(assembly_area + offset, c->data, c->size);
            offset += c->size;
            free_chunk(c);
            continue;
        }
        write(fd, assembly_area, offset);
        offset = 0;
        memcpy(assembly_area + offset, c->data, c->size);
        free_chunk(c);
    }
    close(fd);
}

static void* assemble_thread(void* argv) {
    printf("[debug] client assemble_thread %s.\n", (char*)argv);
    char* path = (char*)argv;
    do_assemble(path);
    return NULL;
}

void start_assemble_phase(const char *path) {
    printf("[debug] client start_assemble_phase %s.\n", path);
    pthread_create(&assemble_t, NULL, assemble_thread, (void *)path);
}

void stop_assemble_phase(void) {
    pthread_join(assemble_t, NULL);
}