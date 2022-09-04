#include "Group.hh"

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
    char *ret = (char *)malloc(sizeof(char) * result.size() + 1);
    strcpy(ret, result.c_str());
    return ret;
}

struct group* new_group(char* file, int len) {
    // TO DO:
}

void delete_group(struct group* gp) {
    // TO DO:
}

vector<struct group*> split2Groups(int fd) {
    // TO DO:
}

int getNodeId(struct group* gp) {
    // TO DO:
}



