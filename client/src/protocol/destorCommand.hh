#ifndef _DESTORCOMMAND_HH_
#define _DESTORCOMMAND_HH_

#include "../inc/include.hh"
#include "../util/RedisUtil.hh"

/*
 * destor Command format
 * request type:
 *      
 * 
 * 
 */ 

class destorCommand
{
private:
    /* data */
    char *_destorCmd;
    int _cmLen;
    std::string _rKey;
    int _type;
    unsigned int _clientIp;

public:
    destorCommand(/* args */);
    destorCommand(char* reqStr);
    ~destorCommand();

    // basic construction methods
    void writeInt(int value);
    void writeString(std::string s);
    int readInt();
    int readRawInt();
    std::string readString();

    int getType();
    unsigned int getClientip();

    // send method
    void sendTo(unsigned int ip);
    void sendTo(redisContext* sendCtx);

    // 
    void buildType0(int type,
                    char* groupName,
                    unsigned char* data);
};

#endif