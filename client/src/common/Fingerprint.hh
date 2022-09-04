#ifndef _FINGERPRINT_HH_
#define _FINGERPRINT_HH_

#include "../inc/include.hh"
#include "Config.hh"
#include <openssl/sha.h>

typedef unsigned char fingerprint[FP_LENGTH];

fingerprint sha1(unsigned char *data, int32_t size);
uint64_t consistentHash(fingerprint fp, int num);
int fp2Int(fingerprint fp);

#endif