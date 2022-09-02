#ifndef _WORKER_HH_
#define _WORKER_HH_

#include "../inc/include.hh"
#include "Config.hh"
#include "../protocol/AgentCommand.hh"
#include "../protocol/destorCommand.hh"

class Worker
{
private:
    /* data */
    Config* _conf;

    redisContext* _processCtx;
    redisContext* _localCtx;
    redisContext* _destorCtx;
public:
    Worker(Config* conf);
    ~Worker();
    void doProcess();

    void clientWrite(AgentCommand *agCmd);
};




#endif