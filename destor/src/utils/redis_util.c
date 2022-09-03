#include "redis_util.h"

char* ip2Str(unsigned int ip) {
  struct in_addr addr = {ip};
  return inet_ntoa(addr);
}

redisContext* createContext(unsigned int ip) {
  return createContext(ip2Str(ip), 6379);
}

redisContext* createContext(char* ip) {
  return createContext(ip, 6379);
}

redisContext* createContext(char* ip, int port) {
  redisContext* retVal = redisConnect(ip, port);
  if (retVal == NULL || retVal -> err) {
    if (retVal) {
      cerr << "Error: " << retVal -> errstr << endl;
      redisFree(retVal);
    } else {
      cerr << "redis context creation error" << endl;
    }
    throw REDIS_CREATION_FAILURE;
  }
  return retVal;
}

double duration(struct timeval t1, struct timeval t2) {
  return (t2.tv_sec-t1.tv_sec) * 1000.0 + (t2.tv_usec-t1.tv_usec) / 1000.0;
}

void destor_cmd_init(destor_cmd *cmd) {
    cmd->_destorCmd = (char*)calloc(MAX_COMMAND_LEN, sizeof(char));
    cmd->_cmLen = 0;
    cmd->_rKey = "destor_request";
}
void agent_cmd_init(agent_cmd *cmd) {
    cmd->_agCmd = (char*)calloc(MAX_COMMAND_LEN, sizeof(char));
    cmd->_cmLen = 0;
    cmd->_rKey = "dup_agent_request";
}

void destor_cmd_init_with_reqstr(destor_cmd *cmd, char* reqstr)
{
    cmd->_destorCmd = reqstr;
    cmd->_cmLen = 0;

    cmd->_type = destor_cmd_read_int();

    switch(cmd->_type) {
        case 0: ; break;
        default: break;
    }
    cmd->_destorCmd = NULL;
    cmd->_cmLen = 0;
}

void agent_cmd_init_with_reqstr(agent_cmd *cmd, char* reqstr){
    cmd->_destorCmd = reqstr;
    cmd->_cmLen = 0;

    cmd->_type = agent_cmd_read_int();

    switch(cmd->_type) {
        case 0: ; break;
        default: break;
    }
    cmd->_destorCmd = NULL;
    cmd->_cmLen = 0;

}

void destor_cmd_write_int(destor_cmd *cmd, int value) {
  int tmpv = htonl(value);
  memcpy(cmd->_destorCmd + cmd->_cmLen, (char*)&tmpv, 4); cmd->_cmLen += 4;
}

void writeagent_cmd_write_intInt(agent_cmd *cmd, int value) {
  int tmpv = htonl(value);
  memcpy(cmd->_agCmd + cmd->_cmLen, (char*)&tmpv, 4); cmd->_cmLen += 4;
}

void destor_cmd_write_string(destor_cmd *cmd, char* s) {
  int slen = strlen(s);
  int tmpslen = htonl(slen);
  // string length
  memcpy(cmd->_destorCmd + cmd->_cmLen, (char*)&tmpslen, 4); cmd->__cmLen += 4;
  // string
  memcpy(cmd->_destorCmd + cmd->_cmLen, s, slen); cmd->_cmLen += slen;
}

void agent_cmd_write_string(agent_cmd *cmd, char* s) {
  int slen = strlen(s);
  int tmpslen = htonl(slen);
  // string length
  memcpy(cmd->_agCmd + cmd->_cmLen, (char*)&tmpslen, 4); cmd->__cmLen += 4;
  // string
  memcpy(cmd->_agCmd + cmd->_cmLen, s, slen); cmd->_cmLen += slen;
}

int destor_cmd_read_int(destor_cmd *cmd) {
  int tmpint;
  memcpy((char*)&tmpint, cmd->_destorCmd + cmd->_cmLen, 4); cmd->_cmLen += 4;
  return ntohl(tmpint);
}

int agent_cmd_read_int(agent_cmd *cmd) {
  int tmpint;
  memcpy((char*)&tmpint, cmd->_agCmd + cmd->_cmLen, 4); cmd->_cmLen += 4;
  return ntohl(tmpint);
}

char* destor_cmd_read_string(destor_cmd *cmd) {
  char* toret;
  int slen = destor_cmd_read_int(cmd);
  char* toret = (char*)calloc(sizeof(char), slen+1);
  memcpy(toret, cmd->_destorCmd + cmd->_cmLen, slen); cmd->_cmLen += slen;
  return toret;
}

char* agent_cmd_read_string(agent_cmd *cmd) {
  char* toret;
  int slen = agent_cmd_read_int(cmd);
  char* toret = (char*)calloc(sizeof(char), slen+1);
  memcpy(toret, cmd->_agCmd + cmd->_cmLen, slen); cmd->_cmLen += slen;
  return toret;
}