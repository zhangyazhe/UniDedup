#include "redis_util.h"

char* ip2Str(unsigned int ip) {
  struct in_addr addr = {ip};
  return inet_ntoa(addr);
}

uint64_t redis_util_htonll(uint64_t val) {
  return ((uint64_t) htonl(val) << 32) + htonl(val >> 32);
}

uint64_t redis_util_ntohll(uint64_t val) {
  return ((uint64_t) ntohl(val) << 32) + ntohl(val >> 32);
}


redisContext* createContextByUint(unsigned int ip) {
  return createContext(ip2Str(ip), 6379);
}

redisContext* createContextByChar(char* ip) {
  return createContext(ip, 6379);
}

redisContext* createContext(char* ip, int port) {
  redisContext* retVal = redisConnect(ip, port);
  // printf("destor-retval-err: %d\n", retVal->err);
  if (retVal == NULL || retVal -> err) {
    if (retVal) {
      // cerr << "Error: " << retVal -> errstr << endl;
      printf("Error: %s\n", retVal->errstr);
      redisFree(retVal);
    } else {
      // cerr << "redis context creation error" << endl;
      printf("redis context creation error\n");
    }
    // throw REDIS_CREATION_FAILURE;
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

void openec_agent_cmd_init(agent_cmd *cmd){
    cmd->_agCmd = (char*)calloc(MAX_COMMAND_LEN, sizeof(char));
    cmd->_cmLen = 0;
    cmd->_rKey = "ag_request";
}

void destor_cmd_init_with_reqstr(destor_cmd *cmd, char* reqstr)
{
    cmd->_destorCmd = reqstr;
    cmd->_cmLen = 0;

    cmd->_type = destor_cmd_read_int(cmd);

    switch(cmd->_type) {
        case 0: resolve_destor_command_type0(cmd); break;
        case 1: resolve_destor_command_type1(cmd); break;
        default: break;
    }
    cmd->_rKey = "destor_request";
    cmd->_destorCmd = NULL;
    cmd->_cmLen = 0;
}

// void free_destor_cmd(destor_cmd* cmd){
//   switch (cmd->_type)
//   {
//   case 0:
//     free(cmd->_group_name);
//     break;
//   case 1:
//     free(cmd->_to_read_filename);
//   default:
//     break;
//   }
//   free(cmd->_destorCmd);
//   free(cmd->_rKey);
// }

void free_destor_cmd(destor_cmd* cmd){
  switch (cmd->_type)
  {
  case 0:
    free(cmd->_group_name);
    break;
  case 1:
    free(cmd->_to_read_filename);
  default:
    break;
  }
  // if (cmd->_rKey != NULL) free(cmd->_rKey);
  if (cmd->_destorCmd != NULL)free(cmd->_destorCmd);
  
  free(cmd);
}

void free_openec_agent_cmd(agent_cmd* cmd) {
  switch (cmd->_type)
  {
  case 0:
    free(cmd->_filepath);
    free(cmd->_mode);
    free(cmd->_ecidpool);
    break;
  case 1:
    free(cmd->_filepath);
  default:
    break;
  }
  // if (cmd->_rKey != NULL) free(cmd->_rKey);
  if (cmd->_agCmd != NULL)free(cmd->_agCmd);
  free(cmd);
}


void agent_cmd_init_with_reqstr(agent_cmd *cmd, char* reqstr){
    cmd->_agCmd = reqstr;
    cmd->_cmLen = 0;

    cmd->_type = agent_cmd_read_int(cmd);

    switch(cmd->_type) {
        case 0: ; break;
        default: break;
    }
    cmd->_agCmd = NULL;
    cmd->_cmLen = 0;

}

void destor_cmd_write_int(destor_cmd *cmd, int value) {
  int tmpv = htonl(value);
  memcpy(cmd->_destorCmd + cmd->_cmLen, (char*)&tmpv, 4); cmd->_cmLen += 4;
}

void destor_cmd_write_uint(destor_cmd *cmd, uint32_t value) {
  uint32_t tmpv = htonl(value);
  memcpy(cmd->_destorCmd + cmd->_cmLen, (char*)&tmpv, 4); cmd->_cmLen += 4;
}

void agent_cmd_write_int(agent_cmd *cmd, int value) {
  int tmpv = htonl(value);
  memcpy(cmd->_agCmd + cmd->_cmLen, (char*)&tmpv, 4); cmd->_cmLen += 4;
}

void destor_cmd_write_string(destor_cmd *cmd, char* s) {
  int slen = strlen(s);
  int tmpslen = htonl(slen);
  // string length
  memcpy(cmd->_destorCmd + cmd->_cmLen, (char*)&tmpslen, 4); cmd->_cmLen += 4;
  // string
  memcpy(cmd->_destorCmd + cmd->_cmLen, s, slen); cmd->_cmLen += slen;
}

void agent_cmd_write_string(agent_cmd *cmd, char* s) {
  int slen = strlen(s);
  int tmpslen = htonl(slen);
  // string length
  memcpy(cmd->_agCmd + cmd->_cmLen, (char*)&tmpslen, 4); cmd->_cmLen += 4;
  // string
  memcpy(cmd->_agCmd + cmd->_cmLen, s, slen); cmd->_cmLen += slen;
}

int destor_cmd_read_int(destor_cmd *cmd) {
  int tmpint;
  memcpy((char*)&tmpint, cmd->_destorCmd + cmd->_cmLen, 4); cmd->_cmLen += 4;
  return ntohl(tmpint);
}

uint32_t destor_cmd_read_uint(destor_cmd *cmd) {
  uint32_t tmpint;
  memcpy((char*)&tmpint, cmd->_destorCmd + cmd->_cmLen, 4); cmd->_cmLen += 4;
  return ntohl(tmpint);
}

int agent_cmd_read_int(agent_cmd *cmd) {
  int tmpint;
  memcpy((char*)&tmpint, cmd->_agCmd + cmd->_cmLen, 4); cmd->_cmLen += 4;
  return ntohl(tmpint);
}

char* destor_cmd_read_string(destor_cmd *cmd) {
  int slen = destor_cmd_read_int(cmd);
  char* toret = (char*)calloc(sizeof(char), slen+1);
  memcpy(toret, cmd->_destorCmd + cmd->_cmLen, slen); cmd->_cmLen += slen;
  return toret;
}

char* agent_cmd_read_string(agent_cmd *cmd) {
  int slen = agent_cmd_read_int(cmd);
  char* toret = (char*)calloc(sizeof(char), slen+1);
  memcpy(toret, cmd->_agCmd + cmd->_cmLen, slen); cmd->_cmLen += slen;
  return toret;
}


void openec_agent_cmd_send_to(agent_cmd* cmd, unsigned int ip) {
  // printf("[debug] send to oec.\n");
  redisContext* sendCtx = createContextByUint(ip);
  redisReply* rReply = (redisReply*)redisCommand(sendCtx, "RPUSH %s %b", cmd->_rKey, cmd->_agCmd, cmd->_cmLen);
  freeReplyObject(rReply);
  redisFree(sendCtx);
  // printf("[debug] send to oec finished.\n");
}


void build_destor_command_type0(destor_cmd* cmd, int type, char* group_name, uint32_t size)
{
	cmd->_type = type;
	int group_name_len = strlen(group_name);
  // int data_len = strlen(data);
  cmd->_group_name = (char*)calloc(sizeof(char), group_name_len+1);
  // cmd->_data = (char*)calloc(sizeof(char), data_len+1);
  strcpy(cmd->_group_name, group_name);
  // strcpy(cmd->_data, data);
	// 1. type
	destor_cmd_write_int(cmd, cmd->_type);
	// 2. group_name
	destor_cmd_write_string(cmd, cmd->_group_name);
	// 3. data
	// destor_cmd_write_string(cmd, cmd->_data);
  // 4. size
  destor_cmd_write_uint(cmd, cmd->_size);
}

void resolve_destor_command_type0(destor_cmd* cmd)
{
  cmd->_group_name = destor_cmd_read_string(cmd);
  // cmd->_data = destor_cmd_read_string(cmd);
  cmd->_size = destor_cmd_read_uint(cmd);
}

void build_destor_command_type1(destor_cmd* cmd, int type, char* filename, unsigned int client_ip)
{
	cmd->_type = type;
	int filename_len = strlen(filename);
  cmd->_to_read_filename = (char*)calloc(sizeof(char), filename_len+1);
  strcpy(cmd->_to_read_filename, filename);
  cmd->_client_ip = client_ip;
	// 1. type
	destor_cmd_write_int(cmd, cmd->_type);
	// 2. filename
  destor_cmd_write_string(cmd, cmd->_to_read_filename);
  // 3. clientip
  destor_cmd_write_uint(cmd, cmd->_client_ip);
}

void resolve_destor_command_type1(destor_cmd* cmd)
{
  cmd->_to_read_filename = destor_cmd_read_string(cmd);
  cmd->_client_ip = destor_cmd_read_uint(cmd);
}

void build_openec_agent_command_type0(agent_cmd* cmd, int type, char* filename, char* ecidpool, char* mode, int filesize)
{
  cmd->_type = type;
  int filename_len = strlen(filename);
  int ecidpool_len = strlen(ecidpool);
  int mode_len = strlen(mode);
  cmd->_filepath = (char*)calloc(filename_len+1, 1);
  cmd->_ecidpool = (char*)calloc(ecidpool_len+1, 1);
  cmd->_mode = (char*)calloc(mode_len+1, 1);
  strcpy(cmd->_filepath, filename);
  strcpy(cmd->_ecidpool, ecidpool);
  strcpy(cmd->_mode, mode);
  cmd->_filesize = filesize;
  // 1. type
  agent_cmd_write_int(cmd, cmd->_type);
  // 2. filename
  agent_cmd_write_string(cmd, cmd->_filepath);
  // 3. ecidpool
  agent_cmd_write_string(cmd, cmd->_ecidpool);
  // 4. mode
  agent_cmd_write_string(cmd, cmd->_mode);
  // 5. filesize
  agent_cmd_write_int(cmd, cmd->_filesize);
}

void build_openec_agent_command_type1(agent_cmd* cmd, int type, char* filename) {
  cmd->_type = type;
  int filename_len = strlen(filename);
  cmd->_filepath = (char*)malloc(filename_len+1);
  strcpy(cmd->_filepath, filename);
  // 1. type
  agent_cmd_write_int(cmd, cmd->_type);
  // 2. filename
  agent_cmd_write_string(cmd, cmd->_filepath);
}
