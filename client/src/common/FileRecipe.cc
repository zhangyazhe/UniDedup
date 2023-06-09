#include "FileRecipe.hh"

std::unordered_map<string, struct fileRecipe*> name2FileRecipe;

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
    fr->gm = (groupMeta *) malloc(num * sizeof(groupMeta));

    return fr;
}

void delete_fileRecipe(struct fileRecipe* fr) {
    if(fr == NULL) return;
    free(fr->filename);
    for(int i = 0; i < fr->num; i++)
        free(fr->gm[i].groupName);
    free(fr->gm);
    free(fr);
}

struct fileRecipe* genFileRecipe(const char* filename, vector<struct group*>& gps) {
    size_t gps_size = gps.size();
    struct fileRecipe* fr = new_fileRecipe(filename, gps_size);
    
    for(int i = 0; i < gps_size; i++) {
        struct group* gp = gps[i];
        size_t nameLen = strlen(gp->groupName);
        fr->gm[i].groupName = (char *) malloc((nameLen+1) * sizeof(char));
        memcpy(fr->gm[i].groupName, gp->groupName, nameLen+1);
        fr->gm[i].nodeId = gp->nodeId;
    }
    return fr;
}

int setFileRecipe(struct fileRecipe* fr) {
    name2FileRecipe[string(fr->filename)] = fr;
    return 0;
    // return setByEchash(ECH, convertType(fr));
}

struct fileRecipe* getFileRecipe(string filename) {
    if(name2FileRecipe.find(filename) == name2FileRecipe.end()) {
        printf("file recipe not found\n");
        return nullptr;
    }
    return name2FileRecipe[filename];
    // return getByEchash(ECH, filename.c_str());
    
}