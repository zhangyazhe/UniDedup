#include "destor.h"
#include <hiredis/hiredis.h>

void destor_server_process()
{
    // todo: where to get local ip
    _localCtx = createContext(/* local_ip */);
    redisReply *rReply;
    while (true)
    {
        printf("destor_server_process\n");
        // will never stop looping
        rReply = (redisReply *)redisCommand(_localCtx, "blpop coor_request 0");
        if (rReply->type == REDIS_REPLY_NIL)
        {
            printf("destor_server_process: get feed back empty queue\n");
        }
        else if (rReply->type == REDIS_REPLY_ERROR)
        {
            printf("destor_server_process: get feed back ERROR happens\n");
        }
        else
        {
            printf("destor_server_process: receive a request!");
            char *reqStr = rReply->element[1]->str;
            destor_cmd *cmd = (destor_cmd *)calloc(sizeof(destor_cmd, 1));
            destor_cmd_init_with_reqstr(cmd, reqStr);
            int type = cmd->_type;
            printf("type: %d \n", type);
            switch (type)
            {
            case 0:;
                break;
            default:
                break;
            }
            free(cmd);
        }
        // free reply object
        freeReplyObject(rReply);
    }
}