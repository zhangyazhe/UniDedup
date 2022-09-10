#ifndef _WORKER_HH_
#define _WORKER_HH_

#include <assert.h>
#include "../inc/include.hh"
#include "Config.hh"
#include "../protocol/AgentCommand.hh"
#include "../protocol/destorCommand.hh"
#include "Group.hh"
#include "Fingerprint.hh"
#include "FileRecipe.hh"
#include "Chunking.hh"

class Worker
{
private:
    /* data */
    Config* _conf;

    redisContext* _processCtx;
    redisContext* _localCtx;
    // redisContext* _destorCtx;
public:
    Worker(Config* conf);
    ~Worker();
    void doProcess();

    void clientWrite(AgentCommand *agCmd);
};




#endif