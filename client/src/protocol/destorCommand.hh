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

	// type0
	std::string _groupName;
    uint32_t _size;
    // type1
    std::string _filename;
    unsigned int _localIp;

public:
    destorCommand(/* args */);
    destorCommand(char* reqStr);
    ~destorCommand();

    // basic construction methods
    void writeInt(int value);
    void writeUInt(uint32_t value);
    void writeString(std::string s);
    int readInt();
    uint32_t readUInt();
    int readRawInt();
    std::string readString();

    int getType();
    unsigned int getClientip();

    // send method
    void sendTo(unsigned int ip);
    void sendTo(redisContext* sendCtx);

    // build command
    void buildType0(int type,
                    std::string groupName,
                    uint32_t size);
    void buildType1(int type,
                    std::string filename,
                    unsigned int ip);
    // resolve command
    void resolveType0();
    void resolveType1();
};

#endif