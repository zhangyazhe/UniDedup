#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <fstream>

using namespace std;


vector<string> split(const string& str, const string& delim) {
	vector<string> res;
	if("" == str) return res;
	//先将要切割的字符串从string类型转换为char*类型
	char * strs = new char[str.length() + 1] ;
	strcpy(strs, str.c_str()); 
 
	char * d = new char[delim.length() + 1];
	strcpy(d, delim.c_str());
 
	char *p = strtok(strs, d);
	while(p) {
		string s = p; //分割得到的字符串转换为string类型
		res.push_back(s); //存入结果数组
		p = strtok(NULL, d);
	}
 
	return res;
}

long long readTotal(string filename) {
    fstream fs(filename.c_str());
    if (!fs.is_open()) {
        cout << "open " << filename << " failed" << endl;
        exit(-1);
    }
    string line;
    long long ret = 0;
    while (getline(fs, line)) {
        if (line.find("total size") != string::npos) {
            vector<string> vec = split(line, " ");
            ret += stoi(vec[vec.size()-1]);
        }
    }
    return ret;
}
long long readStored(string filename) {
    fstream fs(filename.c_str());
    if (!fs.is_open()) {
        cout << "open " << filename << " failed" << endl;
        exit(-1);
    }
    string line;
    long long ret = 0;
    while (getline(fs, line)) {
        if (line.find("stored data size") != string::npos) {
            vector<string> vec = split(line, " ");
            ret += stoi(vec[vec.size()-1]);
        }
    }
    return ret;
}

int main() {
    string filename_prefix = "/home/openec/Fast23/utils/log/destor_";
    vector<string> name{"node01.log", "node02.log", "node03.log"};
    long long readTotalSum = 0, readStoredSum = 0;
    for (int i = 0; i < name.size(); i++) {
        string filename = filename_prefix + name[i];
        readTotalSum += readTotal(filename);
        readStoredSum += readStored(filename);
    }
    cout << "dedup radio = " << ((double)readTotalSum - (double)readStoredSum) / (double)readTotalSum << " ";
    cout << (double)readTotalSum / (double)readStoredSum << endl;
    return 0;
}