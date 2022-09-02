#include "Logger.hh"

// Logger::Logger(std::string& filePath)
// {
    
// }

Logger::~Logger()
{
}

void Logger::logRepairTime
(const std::string &ecid, const std::string &ecclass, double repairtime)
{
    writer.open(log_path, ios::out | ios::app);
    std::string outinfo;
    outinfo = ecid+string(" ")+ecclass+string(" ")+to_string(repairtime);
    writer << outinfo << std::endl;
    writer.close();
}