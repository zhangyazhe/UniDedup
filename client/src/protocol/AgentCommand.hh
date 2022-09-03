#ifndef _AGENTCOMMAND_HH_
#define _AGENTCOMMAND_HH_

#include "../inc/include.hh"
#include "../util/RedisUtil.hh"

/*
 * Agent Command format
 * request type:
 *      type = 0 (client write file) | filepath | sizeinBytes
 *
 * 
 * 
 */ 


class AgentCommand
{
private:
    /* data */
    char *_agCmd = 0;
    int _cmLen = 0;

    std::string _rKey;

    int _type;

    // type 0
    std::string _filepath;
    int _filesize;
public:
    AgentCommand(/* args */);
    AgentCommand(char* reqStr);
    ~AgentCommand();

    // basic methods
    void writeInt(int value);
    void writeString(std::string s);
    int readInt();
    std::string readString();

    int getType();
    char* getCmd();
    int getCmdLen();
    std::string getFilename();
    int getFilesize();

    // send method
    void setRKey(std::string key);
    void sendTo(unsigned int ip);

    // build AgentCommand
    void buildType0(int type,
                    std::string filepath,
                    int filesize);
    

    // resolve AgentCommand
    void resolveType0();

};



#endif