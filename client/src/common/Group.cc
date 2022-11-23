#include "Group.hh"

SyncQueue *read_queue;
SyncQueue *chunk_queue;
SyncQueue *hash_queue;

int openFile(const char* path) {
    int fd = open(path, O_RDONLY);
    unsigned char* buf[10];
    if (fd < 0) {
        fprintf(stderr, "openFile: open %s failed, errno is %d\n", path, errno);
        return -1;
    }
    return fd;
}

char* baseName(const char* filepath){
    std::string str(filepath);
    int len = str.length();
    int end = len - 1;
    while (str[end] == ' ') {
        --end;
    }
    size_t begin = str.rfind('/');
    begin = (begin == std::string::npos) ? 0 : begin + 1;
    std::string result = str.substr(begin, end - begin + 1);
    char *ret = (char *)malloc(sizeof(char) * (result.size() + 1));
    if (ret == NULL) {
        fprintf(stderr, "baseName: memory allocate failed\n");
        return NULL;
    }
    strcpy(ret, result.c_str());
    return ret;
}

struct group* new_group(const char* fileName, int size, int id) {
    struct group *grp = (struct group*)malloc(sizeof(struct group));
    if (grp == NULL) {
        fprintf(stderr, "new_group: memory allocate failed\n");
        return NULL;
    }
    
    grp->size = size;
    grp->data = (unsigned char*)malloc(sizeof(unsigned char) * size);
    if (grp->data == NULL) {
        free(grp);
        fprintf(stderr, "new_group: memory allocate failed\n");
        return NULL;
    }
    std::string groupNameStr = string(fileName) + to_string(id);
    grp->groupName = (char *)malloc(sizeof(char) * (groupNameStr.length() + 1));
    if (grp->groupName == NULL) {
        free(grp->data);
        free(grp);
        fprintf(stderr, "new_group: memory allocate failed\n");
        return NULL;
    }
    strcpy(grp->groupName, groupNameStr.c_str());
    return grp;
}

void delete_group(struct group* gp) {
    if (gp == NULL) return;
    free(gp->groupName);
    free(gp->data);
    free(gp);
}

static inline int my_cmp(fingerprint fp1, fingerprint fp2) {
    for (int i = 0; i < FP_LENGTH; ++i) {
        if (fp1[i] < fp2[i]) {
            return -1;
        } else if (fp1[i] > fp2[i]) {
            return 1;
        }
    }
    return 0;
}

std::vector<struct group*> split2Groups(const char* filepath, const char* filename, Config* conf) {
    int nodeNum = conf->node_num;
    int stateful_routing_enabled = conf->stateful_routing_enabled;
    unsigned int local_ip = conf->_localIP;
    int cluster_enabled = conf->redis_cluster_enabled;
    std::vector<struct group*> result;
    int groupCnt = 0;

    // read file
    start_read_phase(filepath);
    // chunking
    start_chunk_phase();
    // hashing
    start_hash_phase();

    // grouping
    // 1. group chunks by grouping_number
    // 2. get delegate fingerprint
    // 3. get node id
    struct chunk* chunkList[DEFAULT_GROUP_SIZE];
    fingerprint minFingerprint;
    for (int i = 0; i < FP_LENGTH; ++i) {
        minFingerprint[i] = UCHAR_MAX;
    }
    uint32_t size = 0;
    int id = 0;
    while (1) {
        struct chunk *c = (struct chunk *) sync_queue_pop(hash_queue);

        if (c == NULL) {
            break;
        }

        assert(c->data != NULL);
        chunkList[groupCnt] = c;
        groupCnt++;
        if (groupCnt == DEFAULT_GROUP_SIZE) {
            for (int i = 0; i < DEFAULT_GROUP_SIZE; ++i) {
                size += chunkList[i]->size;
                // find the least fp and set it to minFingerprint
                if (my_cmp(chunkList[i]->fp, minFingerprint) < 0) {
                    memcpy(minFingerprint, chunkList[i]->fp, sizeof(fingerprint));
                }
            }
            struct group *gp = new_group(filename, size, id++);
            // set minFingerprint to be the delegate of this group
            memcpy(gp->delegate, minFingerprint, sizeof(fingerprint));
            
            if (stateful_routing_enabled == 0) {
                gp->nodeId = consistentHash(gp->delegate, nodeNum);
            } else {
                // prepare sample_unit info for stateful routing
                vector<SampleUnit> sample_unit_list;
                SampleUnit su;
                for (int i = 0; i < DEFAULT_GROUP_SIZE; i++) {
                    if (i % DEFAULT_SAMPLE_UNIT_SIZE == 0) {
                        for (int j = 0; j < FP_LENGTH; ++j) {
                            su.feature[j] = UCHAR_MAX;
                        }
                        su.size = 0;
                    }
                    su.size += chunkList[i]->size;
                    if (my_cmp(chunkList[i]->fp, su.feature) < 0) {
                        memcpy(su.feature, chunkList[i]->fp, sizeof(fingerprint));
                    }
                    if ((i + 1) % DEFAULT_SAMPLE_UNIT_SIZE == 0) {
                        sample_unit_list.push_back(su);
                    }
                }
                gp->nodeId = getNodeForStatefulRouting(sample_unit_list, nodeNum, local_ip, cluster_enabled);
                if (gp->nodeId == -1) {
                    cerr << "[Stateful Routing] getNodeForStatefulRouting failed" << endl;
                }
            }
            int offset = 0;
            for (int i = 0; i < groupCnt; ++i) {
                assert(chunkList[i]->data != NULL);
                memcpy(gp->data + offset, chunkList[i]->data, chunkList[i]->size);
                free(chunkList[i]->data);
                offset += chunkList[i]->size;
            }
            result.push_back(gp);
            size = 0;
            groupCnt = 0;
            for (int i = 0; i < FP_LENGTH; ++i) {
                minFingerprint[i] = UCHAR_MAX;
            }
        }
    }
    // Process the last set of less than 1024 chunks
    if (groupCnt != 0) {
        for (int i = 0; i < groupCnt; ++i) {
            size += chunkList[i]->size;
            if (my_cmp(chunkList[i]->fp, minFingerprint)) {
                memcpy(minFingerprint, chunkList[i]->fp, sizeof(fingerprint));
            }
        }
        struct group *gp = new_group(filename, size, id++);
        memcpy(gp->delegate, minFingerprint, sizeof(fingerprint));
        if (stateful_routing_enabled == 0) {
            gp->nodeId = consistentHash(gp->delegate, nodeNum);
        } else {
            // prepare sample_unit info for stateful routing
            vector<SampleUnit> sample_unit_list;
            SampleUnit su;
            for (int i = 0; i < groupCnt; i++) {
                if (i % DEFAULT_SAMPLE_UNIT_SIZE == 0) {
                    for (int j = 0; j < FP_LENGTH; ++j) {
                        su.feature[j] = UCHAR_MAX;
                    }
                    su.size = 0;
                }
                su.size += chunkList[i]->size;
                if (my_cmp(chunkList[i]->fp, su.feature) < 0) {
                    memcpy(su.feature, chunkList[i]->fp, sizeof(fingerprint));
                }
                if ((i + 1) % DEFAULT_SAMPLE_UNIT_SIZE == 0 || (i + 1) == groupCnt) {
                    sample_unit_list.push_back(su);
                }
            }
            gp->nodeId = getNodeForStatefulRouting(sample_unit_list, nodeNum, local_ip, cluster_enabled);
            if (gp->nodeId == -1) {
                cerr << "[Stateful Routing] getNodeForStatefulRouting failed" << endl;
            }
        }
        int offset = 0;
        for (int i = 0; i < groupCnt; ++i) {
            assert(chunkList[i]->data != NULL);
            memcpy(gp->data + offset, chunkList[i]->data, chunkList[i]->size);
            free(chunkList[i]->data);
            offset += chunkList[i]->size;
        }
        result.push_back(gp);
    }
    stop_read_phase();
    stop_chunk_phase();
    stop_hash_phase();
    return result;
}

