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
    ~destorCommand();

    // basic construction methods
    void writeInt(int value);
    void writeString(string s);
    int readInt();
    int readRawInt();
    string readString();

    int getType();
    unsigned int getClientip();

    // send method
    void sendTo(unsigned int ip);
    void sendTo(redisContext* sendCtx);
};


#endif