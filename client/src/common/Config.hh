#ifndef _CONFIG_HH_
#define _CONFIG_HH_

#include "../inc/include.hh"
#include <unordered_map>
#include <sstream>
#include "Chunking.hh"
#include "Group.hh"

const std::string config_path = "config/config.txt";
const std::string ip_list_path = "config/ip_list.txt";

class Config
{
private:
    void getIpFromIpList(const std::string& ipListPath);
    void getConfigFromLine(std::string& line);
public:
    Config(const std::string& configPath);
    ~Config();
    unsigned int _localIP;

    int worker_num;

    int node_num;
    std::unordered_map<uint64_t, unsigned int> id2Ip;
    int chunking_algorithm;
    int grouping_algorithm;
    int grouping_number;

    int destor_chunk_avg_size;
    int destor_chunk_min_size;
    int destor_chunk_max_size;
};




#endif