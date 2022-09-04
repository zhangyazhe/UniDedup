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

#define DEFAULT_GROUP_SIZE 4*1024*1024

struct group {
    char* groupName;
    fingerprint delegate;
    uint64_t nodeId;
    uint32_t size;
    unsigned char *data;
};

int openFile(const char* path);
char* baseName(const char* filepath);
struct group* new_group(char *filename, int len);
void delete_group(struct group* gp);
vector<struct group*> split2Groups(int fd);
int getNodeId(struct group* gp);

#endif