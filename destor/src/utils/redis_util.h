#ifndef REDIS_UTIL_H_
#define REDIS_UTIL_H_

#include <hiredis/hiredis.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_COMMAND_LEN (4096)
#define OEC_PKTSIZE (32768ll)

typedef struct destor_cmd
{
    char* _destorCmd;
    int _cmLen;
    char* _rKey;
    int _type;
    unsigned int _clientIp;

    // type0
	char* _group_name;
	char* _data;
    uint32_t _size;

} destor_cmd;

typedef struct agent_cmd
{
    char *_agCmd;
    int _cmLen;
    char* _rKey;
    int _type;

    // type 0
    char* _filepath;
    int _filesize;
    // for openec
    char* _ecidpool;
    char* _mode;

} agent_cmd;


char* ip2Str(unsigned int ip);
redisContext* createContextByUint(unsigned int ip);
redisContext* createContextByChar(char* ip);
redisContext* createContext(char* ip, int port);

double duration(struct timeval t1, struct timeval t2);
// basic methods
void destor_cmd_init(destor_cmd *cmd);
void agent_cmd_init(agent_cmd *cmd);
void openec_agent_cmd_init(agent_cmd *cmd);
void destor_cmd_init_with_reqstr(destor_cmd *cmd, char* reqstr);
void agent_cmd_init_with_reqstr(agent_cmd *cmd, char* reqstr);
void destor_cmd_write_int(destor_cmd *cmd, int value);
void agent_cmd_write_int(agent_cmd *cmd, int value);
void destor_cmd_write_string(destor_cmd *cmd, char* s);
void agent_cmd_write_string(agent_cmd *cmd, char* s);
int destor_cmd_read_int(destor_cmd *cmd);
uint32_t destor_cmd_read_uint(destor_cmd *cmd);
int agent_cmd_read_int(agent_cmd *cmd);
char* destor_cmd_read_string(destor_cmd *cmd);
char* agent_cmd_read_string(agent_cmd *cmd);

// destor command
void build_destor_command_type0(destor_cmd* cmd, int type, char* group_name, char* data);
void resolve_destor_command_type0(destor_cmd* cmd);

// openec agent command
void build_openec_agent_command_type0(agent_cmd* cmd, int type, char* filename, char* ecidpool, char* mode, int filesize);




#endif