#ifndef _CONFIG_HH_
#define _CONFIG_HH_

#include "../inc/include.hh"

const std::string config_path = "";

class Config
{
public:
    Config(std::string& configPath);
    ~Config();
    unsigned int _localIP;

};




#endif