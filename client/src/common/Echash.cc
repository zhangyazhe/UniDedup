#include "Echash.hh"

struct ECHash_st *ECH;

int initEchash(void) {
    ECHash_init(&ECH);

    FILE *fin_config;
    char conf[1024];
    if(!(fin_config = fopen("config/echash_config.txt", "r")))
    {
        printf("The config file not exist.\n");
        exit(-1);
    }
    while(fgets(conf, 1024, fin_config) && (!feof(fin_config)))
    {
        char ip[50]={0};
        uint32_t port=0;
        uint32_t ring=0;

        sscanf(conf,"%s %u %u", ip, &port, &ring);
        if(strncmp(ip,"#Scale",6)==0)
            break;

        if(ip[0]=='#' || ip[0]==0)
            continue;

        fprintf(stderr,"%s, %u ,%u\n",ip, port,ring);
        ECHash_init_addserver(ECH, ip, port, ring);
    }
    fclose(fin_config);

    return 1;
}

uint32_t ECH_set(struct ECHash_st *ECH, char *key, uint32_t key_len,char *val, uint32_t value_len) {
    memcached_return_t rc;

    rc = ECHash_set(ECH, key, key_len, val, value_len, (time_t) 0, (uint32_t) 0);

    if (rc != MEMCACHED_SUCCESS)
    {
        return 1;
    }
    return 0;
}

char* ECH_get(struct ECHash_st* ECH, const char* key) {
    memcached_return_t rc = MEMCACHED_FAILURE;
    char* val;
    size_t len;
    uint32_t flag;
    val = ECHash_get(ECH, key, strlen(key), &len, &flag, &rc);

    return val;
}

int setByEchash(struct ECHash_st *ECH, struct echash_set_t* es) {
    return ECH_set(ECH, es->key, es->key_len, es->value, es->value_len);
}

struct echash_set_t* convertType(struct fileRecipe* fr) {
    struct echash_set_t* es = (struct echash_set_t*) malloc(sizeof(struct echash_set_t));

    size_t key_len = strlen(fr->filename);
    es->key = (char *) malloc((key_len+1) * sizeof(char));
    es->key_len = key_len;

    memcpy(es->key, fr->filename, key_len+1);
    
    size_t value_len = sizeof(int32_t);

    for(int i = 0; i < fr->num; i++) {
        value_len += sizeof(uint32_t);
        value_len += strlen(fr->gm[i].groupName)+1;
        value_len += sizeof(uint64_t);
    }

    es->value = (char *) malloc(value_len * sizeof(char));
    es->value_len = value_len;
    
    int fr_num = htonl(fr->num);
    memcpy(es->value, (char*)&fr_num, sizeof(int32_t));

    //memcpy(es->value+sizeof(int32_t)+sizeof(size_t), fr->gm, value_len-sizeof(int32_t)-sizeof(size_t));
    int written_size = sizeof(int32_t);
    for(int i = 0; i < fr->num; i++) {
        uint32_t nameLen = htonl(strlen(fr->gm[i].groupName)+1);
        memcpy(es->value+written_size, (char*)&nameLen, sizeof(uint32_t));
        written_size += sizeof(uint32_t);
        memcpy(es->value+written_size, fr->gm[i].groupName, strlen(fr->gm[i].groupName)+1);
        written_size += strlen(fr->gm[i].groupName)+1;
        uint64_t nodeid = RedisUtil::htonll(fr->gm[i].nodeId);
        memcpy(es->value+written_size, (char*)&nodeid, sizeof(uint64_t));
        written_size += sizeof(uint64_t);
    }

    return es;
}

struct fileRecipe* getByEchash(struct ECHash_st *ECH, const char* key) {
    char* value = ECH_get(ECH, key);
    if(value == NULL) {
        return NULL;
    }

    int readSize = 0;
    uint32_t num = 0;
    memcpy((char*)&num, value+readSize, sizeof(uint32_t));readSize += sizeof(uint32_t);
    num = ntohl(num);

    struct fileRecipe* fr = new_fileRecipe(key, num);

    for(int i = 0; i < num; i++) {
        int32_t nameLen = 0;
        memcpy((char*)&nameLen, value+readSize, sizeof(int32_t));readSize += sizeof(int32_t);
        nameLen = ntohl(nameLen);
        memcpy(fr->gm[i].groupName, value+readSize, nameLen); readSize += nameLen;
        uint64_t nodeId = 0;
        memcpy((char*)&nodeId, value+readSize, sizeof(uint64_t)); readSize += sizeof(uint64_t);
        nodeId = RedisUtil::ntohll(nodeId);
        fr->gm[i].nodeId = nodeId;
    }
    

    return fr;
}