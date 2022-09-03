#ifndef REDIS_UTIL_H_
#define REDIS_UTIL_H_

#include <hiredis/hiredis.h>

typedef struct destor_cmd
{
    char* _destorCmd;
    int _cmLen;
    char* _rKey;
    int _type;
    unsigned int _clientIp;
} destor_cmd;


char* ip2Str(unsigned int ip);
redisContext* createContext(unsigned int ip);
redisContext* createContext(char* ip);
redisContext* createContext(char* ip, int port);

double duration(struct timeval t1, struct timeval t2);



#endif