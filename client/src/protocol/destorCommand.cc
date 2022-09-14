#include "destorCommand.hh"

destorCommand::destorCommand(/* args */)
{
	_destorCmd = (char *)calloc(MAX_COMMAND_LEN, sizeof(char));
	_cmLen = 0;
	_rKey = "destor_request";
}

destorCommand::destorCommand(char *reqStr)
{
	_destorCmd = reqStr;
	_cmLen = 0;

	_type = readInt();

	switch (_type)
	{
	case 0: resolveType0(); break;
	case 1: resolveType1(); break;
	default:
		break;
	}
	_destorCmd = nullptr;
	_cmLen = 0;
}

destorCommand::~destorCommand()
{
	if (_destorCmd)
	{
		free(_destorCmd);
		_destorCmd = 0;
	}
	_cmLen = 0;
}

void destorCommand::writeInt(int value)
{
	int tmpv = htonl(value);
	memcpy(_destorCmd + _cmLen, (char *)&tmpv, 4);
	_cmLen += 4;
}

void destorCommand::writeUInt(uint32_t value)
{
	int tmpv = htonl(value);
	memcpy(_destorCmd + _cmLen, (char *)&tmpv, 4);
	_cmLen += 4;
}

void destorCommand::writeString(std::string s)
{
	int slen = s.length();
	int tmpslen = htonl(slen);
	// string length
	memcpy(_destorCmd + _cmLen, (char *)&tmpslen, 4);
	_cmLen += 4;
	// string
	memcpy(_destorCmd + _cmLen, s.c_str(), slen);
	_cmLen += slen;
}

int destorCommand::readInt()
{
	int tmpint;
	memcpy((char *)&tmpint, _destorCmd + _cmLen, 4);
	_cmLen += 4;
	return ntohl(tmpint);
}

uint32_t destorCommand::readUInt()
{
	uint32_t tmpint;
	memcpy((char *)&tmpint, _destorCmd + _cmLen, 4);
	_cmLen += 4;
	return ntohl(tmpint);
}

int destorCommand::readRawInt()
{
	int tmpint;
	memcpy((char *)&tmpint, _destorCmd + _cmLen, 4);
	_cmLen += 4;
	return tmpint;
}

std::string destorCommand::readString()
{
	std::string toret;
	int slen = readInt();
	char *sname = (char *)calloc(sizeof(char), slen + 1);
	memcpy(sname, _destorCmd + _cmLen, slen);
	_cmLen += slen;
	toret = std::string(sname);
	free(sname);
	return toret;
}

int destorCommand::getType()
{
	return _type;
}

unsigned int destorCommand::getClientip()
{
	return _clientIp;
}

void destorCommand::sendTo(unsigned int ip)
{
	redisContext *sendCtx = RedisUtil::createContext(ip);
	redisReply *rReply = (redisReply *)redisCommand(sendCtx, "RPUSH %s %b", _rKey.c_str(), _destorCmd, _cmLen);
	// if (rReply -> type == REDIS_REPLY_NIL) {
    //   std::cerr << "destorcmd:: get feed back empty queue " << std::endl;
    //   //freeReplyObject(rReply);
    // } else if (rReply -> type == REDIS_REPLY_ERROR) {
    //   std::cerr << "destorcmd:: get feed back ERROR happens " << std::endl;
    // }
	
	freeReplyObject(rReply);
	redisFree(sendCtx);
}

void destorCommand::sendTo(redisContext *sendCtx)
{
	redisReply *rReply = (redisReply *)redisCommand(sendCtx, "RPUSH %s %b", _rKey.c_str(), _destorCmd, _cmLen);
	freeReplyObject(rReply);
}

void destorCommand::buildType0(int type,
							   std::string groupName,
							   uint32_t size)
{
	_type = type;
	_groupName = groupName;
	// _data = data;
	_size = size;

	// 1. type
	writeInt(_type);
	// 2. groupName
	writeString(_groupName);
	// 3. data_size
	writeUInt(size);
}

void destorCommand::resolveType0() {
	_groupName = readString();
	_size = readUInt();
}

void destorCommand::buildType1(int type,
                    std::string filename,
                    unsigned int ip)
{
	_type = type;
	_filename = filename;
	_localIp = ip;

	// 1. type
	writeInt(_type);
	// 2. filename
	writeString(_filename);
	// 3. ip
	writeUInt(_localIp);
}

void destorCommand::resolveType1() {
	_filename = readString();
	_localIp = readUInt();
}