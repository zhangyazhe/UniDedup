#ifndef _FINGERPRINT_HH_
#define _FINGERPRINT_HH_

#include "../inc/include.hh"
#include "Config.hh"

typedef unsigned char fingerprint[20];

fingerprint sha1(unsigned char *data, int32_t size);
fingerprint delegate(struct group* gp);
int64_t consitentHash(fingerprint fp);

#endif