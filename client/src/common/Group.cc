#include "Group.hh"
#include <sstream>

int openFile(const char* path) {
    // TO DO:
    int fd = open(path, O_RDONLY);
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
    int begin = str.rfind('/');
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

struct group* new_group(char* fileName, int size) {
    // TO DO:
    struct group *grp = (struct group*)malloc(sizeof(struct group));
    if (grp == NULL) {
        fprintf(stderr, "new_group: memory allocate failed\n");
        return NULL;
    }
    
    grp->size = size;
    grp->data = (unsigned char*)malloc(sizeof(unsigned char) * len);
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
    // TO DO:
    if (gp == NULL) {
        return ;
    }
    free(gp->groupName);
    free(gp->data);
    free(gp);
}

// 尚未测试！！！ 
vector<struct group*> split2Groups(int fd) {
    // TO DO:
    char fileName[1024] = {'\0'};
    char path[1024] = {'\0'};
    snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);
    readlink(path, fileName, sizeof(fileName) - 1);
    unsigned char buffer[DEFAULT_GROUP_SIZE];
    std::vector<struct group*> groupList;
    ssize_t size = read(fd, buffer, DEFAULT_GROUP_SIZE);
    while (size != -1 && size != 0) {
        struct group *gp = new_group(fileName, size);
        memcpy(gp->data, buffer, size);
        memcpy(gp->delegate, delegate(gp), FP_LENGTH);
        gp->nodeId = consitentHash(gp->delegate);
        groupList.push_back(gp);
    }
    if (size == -1) {
        fprintf(stderr, "split2Groups: read file failed.\n");
        for (int i = 0; i < groupList.size(); ++i) {
            delete_group(groupList[i]);
        }
        groupList.clear();
        return groupList;
    }
    return groupList;
}



