#ifndef _GROUP_HH_
#define _GROUP_HH_

#include "../inc/include.hh"
#include "Config.hh"
#include "Fingerprint.hh"
#include <vector>

#define DEFAULT_GROUP_SIZE 4*1024*1024

struct group {
    uint64_t id;
    uint32_t size;
    fingerprint delegate;
    uint64_t nodeId;
    unsigned char *data;
};

int openFile(const char* path);
char* baseName(const char* filepath);
struct group* new_group(int len);
void delete_group(struct group* gp);
vector<struct group*> split2Groups(int fd);
int getNodeId(struct group* gp);

#endif