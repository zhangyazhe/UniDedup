#ifndef DESTOR_SERVER_H_
#define DESTOR_SERVER_H_

#include "destor.h"
#include <hiredis/hiredis.h>
#include "jcr.h"
#include "utils/sync_queue.h"
#include "index/index.h"
#include "backup.h"
#include "storage/containerstore.h"

static pthread_t read_t;

struct readParam {
    char* data;
    uint32_t size;
};

struct readParam* newReadParam(char* data, uint32_t size);

void deleteReadParam(struct readParam* rp);

// static void read_data(void* argv);

// static void* read_thread(void *argv);

void start_read_phase_from_data(void* argv);

void stop_read_phase_from_data();

void destor_write(char *path, char *data, uint32_t size);

void destor_server_process();

#endif