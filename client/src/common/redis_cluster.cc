#include "redis_cluster.hh"
#include "../util/RedisUtil.hh"
#include <exception>

int setFileRecipeToRedis(struct fileRecipe* fr) {
    try {
        redisContext* redis_ctx = RedisUtil::createContext(_conf->_localIP);
    } catch(exception & e) {
        return 0;
    }

    
}