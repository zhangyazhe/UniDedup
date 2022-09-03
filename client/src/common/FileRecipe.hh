#ifndef _FINGERPRINT_HH_
#define _FINGERPRINT_HH_

#include "../inc/include.hh"
#include "Config.hh"

struct fileRecipe {
    int32_t num;
    int64_t* node_id;
};

struct fileRecipe* getFileRecipe(struct group* gp);
int setFileRecipe(struct fileRecipe* fr);

#endif