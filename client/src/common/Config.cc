#include "Config.hh"

Config::Config(std::string& configPath)
{
    getIpFromIpList(ip_list_path);
}

Config::~Config()
{
}

Config::getIpFromIpList(std::string& ipListPath) {
    // read IPs from ip_list_path
    std::ifstream reader(ipListPath);
    uint64_t idth = 0;
    if (reader.is_open()) {
        std::cout << "Config::read IPs" << std::endl;
        std::string line;
        while (std::getline(reader, line))
        {
            unsigned int iip = inet_addr(line.c_str());
            id2Ip.insert(std::make_pair(idth, iip));
            idth++;
        }
        reader.close();
    }
    
}