#ifndef _FINGERPRINT_HH_
#define _FINGERPRINT_HH_

#include "../inc/include.hh"
#include "Config.hh"
#include <openssl/sha.h>

// typedef unsigned char fingerprint[FP_LENGTH];

void sha1(unsigned char *data, int32_t size, fingerprint fp);
uint64_t consistentHash(fingerprint fp, int num);
uint64_t fp2Int(fingerprint fp);
int fingerprintCmp(fingerprint fp1, fingerprint fp2);

#endif