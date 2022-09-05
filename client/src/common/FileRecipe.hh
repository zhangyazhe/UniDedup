#ifndef _FILERECIPE_HH_
#define _FILERECIPE_HH_

#include "../inc/include.hh"
#include "Config.hh"
#include "Group.hh"
#include "Echash.hh"
#include <vector>

struct groupMeta { 
    char* groupName;
    uint64_t nodeId;
};

struct fileRecipe {
    char* filename;
    int32_t num;
    groupMeta* gm;
};

struct fileRecipe* new_fileRecipe(const char* filename, int num);
void delete_fileRecipe(struct fileRecipe* fr);
struct fileRecipe* getFileRecipe(const char* filename, vector<struct group*>& gps);
int setFileRecipe(struct fileRecipe* fr);

#endif