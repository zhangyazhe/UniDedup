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

uint32_t ECH_set(struct ECHash_st *ECH, char *key, char *val) {
    memcached_return_t rc;

    rc = ECHash_set(ECH, key, strlen(key), val, strlen(val), (time_t) 0, (uint32_t) 0);

    if (rc != MEMCACHED_SUCCESS)
    {
        return 1;
    }
    return 0;
}

int setByEchash(struct ECHash_st *ECH, struct echash_set_t* es) {
    return ECH_set(ECH, es->key, es->value);
}

struct echash_set_t* convertType(struct fileRecipe* fr) {
    struct echash_set_t* es = (struct echash_set_t*) malloc(sizeof(struct echash_set_t));

    size_t key_len = strlen(fr->filename);
    size_t value_len = sizeof(int32_t);
    for(int i = 0; i < fr->num; i++) {
        value_len += strlen(fr->gm[i].groupName)+1;
        value_len += sizeof(uint64_t);
    }
    
    es->key = (char *) malloc((key_len+1) * sizeof(char));
    es->value = (char *) malloc(value_len * sizeof(char));
    
    memcpy(es->key, fr->filename, key_len);
    memcpy(es->value, fr->gm, value_len);
    
    return es;
}