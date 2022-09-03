#ifndef _FINGERPRINT_HH_
#define _FINGERPRINT_HH_

#include "../inc/include.hh"
#include "Config.hh"
#include "Group.hh"
#include <vector>

struct fileRecipe {
    char* filename;
    int32_t num;
    int64_t* node_id;
};

enum echash_return_t {
    ECHASH_SUCCESS,
    ECHASH_FAILURE
}

struct fileRecipe* getFileRecipe(const char* filename, vector<struct group*>& gps);
int setFileRecipe(struct fileRecipe* fr);

#endif