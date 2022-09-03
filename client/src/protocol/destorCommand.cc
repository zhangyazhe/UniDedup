#include "destorCommand.hh"

destorCommand::destorCommand(/* args */)
{
    _destorCmd = (char*)calloc(MAX_COMMAND_LEN, sizeof(char));
    _cmLen = 0;
    _rKey = "destor_request";
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

void destorCommand::writeInt(int value) {
  int tmpv = htonl(value);
  memcpy(_destorCmd + _cmLen, (char*)&tmpv, 4); _cmLen += 4;
}

void destorCommand::writeString(string s) {
  int slen = s.length();
  int tmpslen = htonl(slen);
  // string length
  memcpy(_destorCmd + _cmLen, (char*)&tmpslen, 4); _cmLen += 4;
  // string
  memcpy(_destorCmd + _cmLen, s.c_str(), slen); _cmLen += slen;
}

int destorCommand::readInt() {
  int tmpint;
  memcpy((char*)&tmpint, _destorCmd + _cmLen, 4); _cmLen += 4;
  return ntohl(tmpint);
}

int destorCommand::readRawInt() {
  int tmpint;
  memcpy((char*)&tmpint, _destorCmd + _cmLen, 4); _cmLen += 4;
  return tmpint;
}

std::string destorCommand::readString() {
  std::string toret;
  int slen = readInt();
  char* sname = (char*)calloc(sizeof(char), slen+1);
  memcpy(sname, _destorCmd + _cmLen, slen); _cmLen += slen;
  toret = string(sname);
  free(sname);
  return toret;
}

int destorCommand::getType() {
  return _type;
}

unsigned int destorCommand::getClientip() {
  return _clientIp;
}