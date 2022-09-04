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

#define DEFAULT_GROUP_SIZE 4*1024*1024

struct group {
    char* groupName;
    fingerprint delegate;
    uint64_t nodeId;
    uint32_t size;
    unsigned char *data;
};

struct {
    uint64_t id;
    pthread_mutex_t mutex;
} localId = {0, PTHREAD_MUTEX_INITIALIZER};

int openFile(const char* path);
char* baseName(const char* filepath);
struct group* new_group(char *fileName, int size);
void delete_group(struct group* gp);
vector<struct group*> split2Groups(int fd);

#endif