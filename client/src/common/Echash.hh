#ifndef _ECHASH_HH_
#define _ECHASH_HH_

#include "../libmemcached-1.0.18/libmemcached/memcached.h"
#include "../inc/include.hh"
#include "FileRecipe.hh"

extern struct ECHash_st *ECH;

enum echash_return_t {
    ECHASH_SUCCESS,
    ECHASH_FAILURE
};

struct echash_set_t {
    char *key;
    char *value;
};

int initEchash(void);
int setByEchash(struct ECHash_st *ECH, struct echash_set_t* es);
struct fileRecipe* getByEchash(struct ECHash_st *ECH, const char* key);
struct echash_set_t* convertType(struct fileRecipe* fr);
uint32_t ECH_set(struct ECHash_st *ECH, char *key, char *val);
char* ECH_get(struct ECHash_st* ECH, const char* key);

#endif