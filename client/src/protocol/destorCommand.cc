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
							   std::string data)
{
	_type = type;
	_groupName = groupName;
	_data = data;

	// 1. type
	writeInt(_type);
	// 2. groupName
	writeString(_groupName);
	// 3. data
	writeString(_data);
}

void destorCommand::resolveType0() {
	_groupName = readString();
	_data = readString();
}