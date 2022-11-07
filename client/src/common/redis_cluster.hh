#ifndef _REDIS_CLUSTER_HH_
#define _REDIS_CLUSTER_HH_

#include "Group.hh"
#include "FileRecipe.hh"
#include "../inc/include.hh"

/**
 * @return 0 for success, -1 for fail
*/
int setFileRecipeToRedis(struct fileRecipe* fr, unsigned int local_ip);
struct fileRecipe* getFileRecipeFromRedis(string filename, unsigned int local_ip);

#endif