#ifndef _FILERECIPE_HH_
#define _FILERECIPE_HH_

#include "../inc/include.hh"
#include "Config.hh"
#include "Group.hh"
// #include "Echash.hh"
#include <vector>
#include <unordered_map>
#include <string>

extern std::unordered_map<string, struct fileRecipe*> name2FileRecipe;

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
struct fileRecipe* genFileRecipe(const char* filename, vector<struct group*>& gps);
int setFileRecipe(struct fileRecipe* fr);
struct fileRecipe* getFileRecipe(string filename);

#endif