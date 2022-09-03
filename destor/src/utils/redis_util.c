#include "redis_util.h"

char* ip2Str(unsigned int ip) {
  struct in_addr addr = {ip};
  return inet_ntoa(addr);
}

redisContext* createContext(unsigned int ip) {
  return createContext(ip2Str(ip), 6379);
}

redisContext* createContext(char* ip) {
  return createContext(ip, 6379);
}

redisContext* createContext(char* ip, int port) {
  redisContext* retVal = redisConnect(ip, port);
  if (retVal == NULL || retVal -> err) {
    if (retVal) {
      cerr << "Error: " << retVal -> errstr << endl;
      redisFree(retVal);
    } else {
      cerr << "redis context creation error" << endl;
    }
    throw REDIS_CREATION_FAILURE;
  }
  return retVal;
}

double duration(struct timeval t1, struct timeval t2) {
  return (t2.tv_sec-t1.tv_sec) * 1000.0 + (t2.tv_usec-t1.tv_usec) / 1000.0;
}

