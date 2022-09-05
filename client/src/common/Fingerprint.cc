#include "Fingerprint.hh"

void sha1(unsigned char *data, int32_t size, fingerprint fp); {
    // TO DO:
    fingerprint fp;
    SHA_CTX ctx;
    SHA_Init(&ctx);
	SHA_Update(&ctx, data, size);
	SHA_Final(fp, &ctx);
}

uint64_t consistentHash(fingerprint fp, int num) {
    // TO DO:
    uint64_t hashInt = fp2Int(fp);
    return hashInt % num;
}

uint64_t fp2Int(fingerprint fp) {
    uint64_t res = 0;
    for(int i = 0; i < FP_LENGTH / INT_SIZE; i++) {
        for(int j = 0; j < INT_SIZE; j++) {
            res ^= (uint64_t)fp[j];
            res <<= BYTE_LENGTH;
        }
    }
    return res;
}

int fingerprintCmp(fingerprint fp1, fingerprint fp2) {
    for (int i = 0; i < FP_LENGTH; ++i) {
        if (fp1[i] > fp2[i]) {
            return 1;
        } else if (fp1[i] < fp2[i]) {
            return -1;
        }
    }
    return 0;
}