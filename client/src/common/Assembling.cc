#include "Assembling.hh"

void assemble(const char *path) {
    int fd = -1;
    fd = open(path, O_WRONLY | O_CREAT, S_IRWXU);
    if (fd < 0) {
        fprintf(stderr, "assemble: open file failed\n");
        return ;
    }
    unsigned char assembly_area[ASSEMBLE_BUFFER_LEN] = {'0'};
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

void start_assemble_phase(void) {

}

void stop_assemble_phase(void) {
    
}