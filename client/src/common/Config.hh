#ifndef _CONFIG_HH_
#define _CONFIG_HH_

#include "../inc/include.hh"
#include <unordered_map>

const std::string config_path = "";

class Config
{
public:
    Config(std::string& configPath);
    ~Config();
    unsigned int _localIP;

    unordered_map<uint64_t, unsigned int> id2Ip;
};




#endif