#ifndef _GROUP_HH_
#define _GROUP_HH_

#include "../inc/include.hh"
#include "Config.hh"

struct group {
    uint64_t id;
    uint32_t size;
    unsigned char *data;
};

struct group* new_group(int len);
void delete_group(struct group*);
int split2Groups(int fd, struct group*gps);

#endif