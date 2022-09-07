#include "Group.hh"

SyncQueue *read_queue;
SyncQueue *chunk_queue;
SyncQueue *hash_queue;

int openFile(const char* path) {
    // TO DO:
    cout << "path: " << string(path) << endl;
    int fd = open(path, O_RDONLY);
    unsigned char* buf[10];
    if (fd < 0) {
        fprintf(stderr, "openFile: open %s failed, errno is %d\n", path, errno);
        return -1;
    }
    return fd;
}

char* baseName(const char* filepath){
    // TO DO:
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

struct group* new_group(const char* fileName, int size) {
    // TO DO:
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
    uint64_t id;
    pthread_mutex_lock(&localId.mutex);
    id = localId.id++;
    pthread_mutex_unlock(&localId.mutex);
    std::string groupNameStr(fileName);
    std::ostringstream oss;
    oss << id;
    groupNameStr += oss.str();
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

// without test
// vector<struct group*> split2GroupsFixed(int fd) {
//     char fileName[1024] = {'\0'};
//     char path[1024] = {'\0'};
//     snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);
//     readlink(path, fileName, sizeof(fileName) - 1);
//     unsigned char buffer[DEFAULT_GROUP_SIZE];
//     std::vector<struct group*> groupList;
//     ssize_t size = read(fd, buffer, DEFAULT_GROUP_SIZE);
//     while (size != -1 && size != 0) {
//         struct group *gp = new_group(fileName, size);
//         memcpy(gp->data, buffer, size);
//         memcpy(gp->delegate, delegate(gp), FP_LENGTH);
//         gp->nodeId = consistentHash(gp->delegate);
//         groupList.push_back(gp);
//     }
//     if (size == -1) {
//         fprintf(stderr, "split2Groups: read file failed.\n");
//         for (int i = 0; i < groupList.size(); ++i) {
//             delete_group(groupList[i]);
//         }
//         groupList.clear();
//         return groupList;
//     }
//     return groupList;
// }

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

std::vector<struct group*> split2Groups(const char* filepath, int nodeNum) {
    std::vector<struct group*> result;
    int groupCnt = 0;

    // read file
    start_read_phase(filepath);
    // chunking
    start_chunk_phase();
    // hash
    start_hash_phase();
    //}

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
    
    while (1) {
        struct chunk *c = (struct chunk *) sync_queue_pop(hash_queue);

        if (c == NULL) {
            break;
        }
        chunkList[groupCnt] = c;
        groupCnt++;
        if (groupCnt == DEFAULT_GROUP_SIZE) {
            for (int i = 0; i < DEFAULT_GROUP_SIZE; ++i) {
                size += chunkList[i]->size;
                if (my_cmp(chunkList[i]->fp, minFingerprint) < 0) {
                    memcpy(minFingerprint, chunkList[i]->fp, sizeof(fingerprint));
                }
            }
            struct group *gp = new_group(filepath, size);
            memcpy(gp->delegate, minFingerprint, sizeof(fingerprint));
            gp->nodeId = consistentHash(gp->delegate, nodeNum);
            result.push_back(gp);
            size = 0;
            groupCnt = 0;
            for (int i = 0; i < FP_LENGTH; ++i) {
                minFingerprint[i] = UCHAR_MAX;
            }
        }
    }
    if (groupCnt != 0) {
        for (int i = 0; i < groupCnt; ++i) {
            size += chunkList[i]->size;
            if (my_cmp(chunkList[i]->fp, minFingerprint)) {
                memcpy(minFingerprint, chunkList[i]->fp, sizeof(fingerprint));
            }
        }
        struct group *gp = new_group(filepath, size);
        memcpy(gp->delegate, minFingerprint, sizeof(fingerprint));
        gp->nodeId = consistentHash(gp->delegate, nodeNum);
        result.push_back(gp);
    }
    stop_read_phase();
    stop_chunk_phase();
    stop_hash_phase();
    return result;
}

