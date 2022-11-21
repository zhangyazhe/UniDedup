/**
 * Divide file to different group.
 */
#ifndef _GROUP_HH_
#define _GROUP_HH_

#include "../inc/include.hh"
#include "Config.hh"
#include "Fingerprint.hh"
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <climits>
#include <sstream>
#include "../util/sync_queue.hh"

#include "Chunking.hh"
#include "Reading.hh"
#include "Hashing.hh"
#include "Config.hh"

#include "stateful_routing.hh"

#define DEFAULT_GROUP_SIZE 1024
#define DEFAULT_BLOCK_SIZE 1048576 // 1MB
#define GROUPING_FIXED 0
// each sample_unit has 128 chunks, each group has 8 sample unit
#define DEFAULT_SAMPLE_UNIT_SIZE 128

struct group {
    char* groupName;
    fingerprint delegate;
    uint64_t nodeId;
    uint32_t size;
    unsigned char *data;
};

typedef struct SampleUnit {
    fingerprint feature;
    uint64_t size;
} SampleUnit;

struct {
    uint64_t id;
    pthread_mutex_t mutex;
} localId = {0, PTHREAD_MUTEX_INITIALIZER};

/* Output of read phase. */
extern SyncQueue *read_queue;
/* Output of chunk phase. */
extern SyncQueue *chunk_queue;
/* Output of hash phase. */
extern SyncQueue *hash_queue;


int openFile(const char* path);
/* get the file name behind the last '/' */
char* baseName(const char* filepath);
struct group* new_group(const char *fileName, int size, int id);
void delete_group(struct group* gp);
vector<struct group*> split2GroupsFixed(int fd);
std::vector<struct group*> split2Groups(const char* filepath, const char* filename, int nodeNum, int stateful_routing_enabled, unsigned int local_ip);

#endif