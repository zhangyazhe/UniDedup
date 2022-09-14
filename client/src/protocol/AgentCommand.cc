#include "AgentCommand.hh"

AgentCommand::AgentCommand(/* args */)
{
	_agCmd = (char *)calloc(MAX_COMMAND_LEN, sizeof(char));
	_cmLen = 0;
	_rKey = "dup_agent_request";
}

AgentCommand::AgentCommand(char *reqStr)
{
	_agCmd = reqStr;
	_cmLen = 0;
	_type = readInt();

	switch (_type)
	{
	case 0: resolveType0(); break;
	case 1: resolveType1(); break;

	default: break;
	}
	_agCmd = nullptr;
	_cmLen = 0;
}

AgentCommand::~AgentCommand()
{
	if (_agCmd)
	{
		free(_agCmd);
		_agCmd = 0;
	}
	_cmLen = 0;
}

void AgentCommand::writeInt(int value)
{
	int tmpv = htonl(value);
	memcpy(_agCmd + _cmLen, (char *)&tmpv, 4);
	_cmLen += 4;
}

void AgentCommand::writeString(std::string s)
{
	int slen = s.length();
	int tmpslen = htonl(slen);
	// string length
	memcpy(_agCmd + _cmLen, (char *)&tmpslen, 4);
	_cmLen += 4;
	// string
	memcpy(_agCmd + _cmLen, s.c_str(), slen);
	_cmLen += slen;
}

int AgentCommand::readInt()
{
	int tmpint;
	memcpy((char *)&tmpint, _agCmd + _cmLen, 4);
	_cmLen += 4;
	return ntohl(tmpint);
}

std::string AgentCommand::readString()
{
	std::string toret;
	int slen = readInt();
	char *sname = (char *)calloc(sizeof(char), slen + 1);
	memcpy(sname, _agCmd + _cmLen, slen);
	_cmLen += slen;
	toret = std::string(sname);
	free(sname);
	return toret;
}

int AgentCommand::getType()
{
	return _type;
}

char *AgentCommand::getCmd()
{
	return _agCmd;
}

int AgentCommand::getCmdLen()
{
	return _cmLen;
}

std::string AgentCommand::getFilename()
{
	return _filesaveas;
}

std::string AgentCommand::getFilepath()
{
	return _filepath;
}

int AgentCommand::getFilesize()
{
	return _filesize;
}

void AgentCommand::setRKey(std::string key)
{
	_rKey = key;
}

std::string AgentCommand::getToReadFilename() {
	return _to_read_filename;
}

void AgentCommand::sendTo(unsigned int ip)
{
	redisContext *sendCtx = RedisUtil::createContext(ip);
	redisReply *rReply = (redisReply *)redisCommand(sendCtx, "RPUSH %s %b", _rKey.c_str(), _agCmd, _cmLen);
	freeReplyObject(rReply);
	redisFree(sendCtx);
}

void AgentCommand::buildType0(int type,
							  std::string filepath,
							  std::string saveas,
							  int filesize)
{
	_type = type;
	_filepath = filepath;
	_filesaveas = saveas;
	_filesize = filesize;

	// 1. type
	writeInt(_type);
	// 2. filepath
	writeString(_filepath);
	// 3. saveas
	writeString(_filesaveas);
	// 4. filesize
	writeInt(_filesize);
}

void AgentCommand::resolveType0()
{
	_filepath = readString();
	_filesaveas = readString();
	_filesize = readInt();
}

void AgentCommand::buildType1(int type,
                    std::string filename)
{
	_type = type;
	_to_read_filename = filename;

	// 1. type
	writeInt(_type);
	// 2. filename
	writeString(_to_read_filename);
}

void AgentCommand::resolveType1()
{
	_to_read_filename = readString();
}
