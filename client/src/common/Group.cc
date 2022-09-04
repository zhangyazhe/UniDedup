#include "Group.hh"
#include <sstream>

static pthread_t read_t;

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

// without test
vector<struct group*> split2GroupsFixed(int fd) {
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

static void read_file(int fd) {
    static unsigned char buf[DEFAULT_BLOCK_SIZE];
    int size = 0;

    struct chunk *c = NULL;

    while ((size = read(fd, buf, DEFAULT_BLOCK_SIZE)) != 0) {
        c = new_chunk(size);
        memcpy(c->data, buf, size);
        sync_queue_push(read_queue, c);
    }
}

static void *read_thread(void *argv) {
    int fd = *(int*)argv;
    read_file(fd);
    sync_queue_term(read_queue);
    return NULL;
}

void start_read_phase(int fd) {
    read_queue = sync_queue_new(10);
    pthread_create(&read_t, NULL, read_thread, (void*)&fd);
}

vector<struct group*> split2Groups(int fd) {
    
    
    // read file
    start_read_phase(fd);
    // chunking
    start_chunk_phase();
    // hash

    //}

    // grouping
    // 1. group chunks by grouping_number
    // 2. get delegate fingerprint
    // 3. get node id
}

