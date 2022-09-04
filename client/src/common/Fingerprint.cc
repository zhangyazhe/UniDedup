#include "Fingerprint.hh"

fingerprint sha1(unsigned char *data, int32_t size) {
    // TO DO:
    fingerprint fp;
    SHA_CTX ctx;
    SHA_Init(&ctx);
	SHA_Update(&ctx, data, size);
	SHA_Final(fp, &ctx);

    return fp;
}

uint64_t consistentHash(fingerprint fp, int num) {
    // TO DO:
    int hashInt = fp2Int(fp);
    return hashInt % num;
}

uint64_t fp2Int(fingerprint fp) {
    uint64_t res = 0;
    for(int i = 0; i < FP_LENGTH / INT_SIZE; i++) {
        for(int j = 0; j < INT_SIZE; j++) {
            res ^= fp[j];
            res <<= BYTE_LENGTH;
        }
    }
    return res;
}