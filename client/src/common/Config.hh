#ifndef _CONFIG_HH_
#define _CONFIG_HH_

#include "../inc/include.hh"
#include <unordered_map>
#include "Chunking.hh"
#include "Group.hh"

const std::string config_path = "../config/config.txt";
const std::string ip_list_path = "../config/ip_list.txt";

class Config
{
private:
    getIpFromIpList(std::string& ipListPath);
public:
    Config(std::string& configPath);
    ~Config();
    unsigned int _localIP;

    std::unordered_map<uint64_t, unsigned int> id2Ip;
    int chunking_algorithm;
    int grouping_algorithm;
    int grouping_number;
};




#endif