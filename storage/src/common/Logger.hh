#ifndef _LOGGER_HH_
#define _LOGGER_HH_

#include "../inc/include.hh"

using namespace std;

const std::string log_path = "./log/log.txt";

class Logger
{
private:
    ofstream writer;
    
public:
    // Logger(std::string& filePath);
    ~Logger();
    void logRepairTime(const std::string &ecid, const std::string &ecclass, double repairtime);
};

#endif
