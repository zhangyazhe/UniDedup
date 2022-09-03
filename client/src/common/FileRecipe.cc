#include "FileRecipe.hh"

struct fileRecipe* new_fileRecipe(const char* filename, int num) {
    struct fileRecipe* fr = (struct fileRecipe*) malloc(sizeof(struct fileRecipe));
    size_t fileNameLen = strlen(filename);

    // set filename
    fr->filename = (char *) malloc((fileNameLen+1) * sizeof(char));
    memcpy(fr->filename, filename, fileNameLen);
    fr->filename[fileNameLen] = '\0';

    // set number of groups
    fr->num = num;

    // malloc node_id;
    fr->nodeId = (uint64_t *) malloc(num * sizeof(uint64_t));

    return fr;
}

void delete_fileRecipe(struct fileRecipe* fr) {
    free(fr->filename);
    free(fr->nodeId);
    free(fr);
}

struct fileRecipe* getFileRecipe(const char* filename, vector<struct group*>& gps) {
    size_t gps_size = gps.size();
    struct fileRecipe* fr = new_fileRecipe(filename, gps_size);
    for(size_t i = 0; i < gps_size; i++) {
        fr->nodeId[i] = gps[i]->nodeId;
    }
    return fr;
}

int setFileRecipe(struct fileRecipe* fr) {
    return setByEchash(ECH, convertType(fr));
}