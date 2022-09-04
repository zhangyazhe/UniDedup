#ifndef _FINGERPRINT_HH_
#define _FINGERPRINT_HH_

#include "../inc/include.hh"
#include "Config.hh"

typedef unsigned char fingerprint[FP_LENGTH];

fingerprint sha1(unsigned char *data, int32_t size);
fingerprint delegate(struct group* gp);
uint64_t consitentHash(fingerprint fp);

#endif