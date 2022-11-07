#ifndef _REDIS_CLUSTER_HH_
#define _REDIS_CLUSTER_HH_

#include "Group.hh"
#include "FileRecipe.hh"
#include "../inc/include.hh"

int setFileRecipeToRedis(struct fileRecipe* fr);
struct fileRecipe* getFileRecipeFromRedis(string filename);

#endif