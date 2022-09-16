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
    std::string _filesaveas;
    int _filesize;
    // type 1
    std::string _to_read_filename;
    std::string _to_save_as;
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
    std::string getFilepath();
    std::string getFilename();
    int getFilesize();
    std::string getToReadFilename();
    std::string getToSaveAs();

    // send method
    void setRKey(std::string key);
    void sendTo(unsigned int ip);

    // build AgentCommand
    void buildType0(int type,
                    std::string filepath,
                    std::string saveas,
                    int filesize);
    void buildType1(int type,
                    std::string filename,
                    std::string saveas);

    // resolve AgentCommand
    void resolveType0();
    void resolveType1();

};



#endif