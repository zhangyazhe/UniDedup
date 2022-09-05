#include "destor.h"
#include <hiredis/hiredis.h>
#include "jcr.h"
#include "utils/sync_queue.h"
#include "index/index.h"
#include "backup.h"
#include "storage/containerstore.h"

static void read_data(char* data) {
	sds filename = sdsdup(jcr.path);

    if (jcr.path[sdslen(jcr.path) - 1] == '/') {
		/* the backup path points to a direcory */
		sdsrange(filename, sdslen(jcr.path), -1);
	} else {
		/* the backup path points to a file */
		int cur = sdslen(filename) - 1;
		while (filename[cur] != '/')
			cur--;
		sdsrange(filename, cur, -1);
	}

    struct chunk *c = new_chunk(sdslen(filename) + 1);
	strcpy(c->data, filename);

	VERBOSE("Read phase: %s", filename);

	SET_CHUNK(c, CHUNK_FILE_START);

	sync_queue_push(read_queue, c);

	TIMER_DECLARE(1);
	TIMER_BEGIN(1);
	int size = 0;

    
}

static void* read_thread(void *argv) {
	/* Each file will be processed separately */
	read_data((char *)data);
	sync_queue_term(read_queue);
	return NULL;
}

void start_read_phase_from_data(char* data) {
    /* running job */
    jcr.status = JCR_STATUS_RUNNING;
	read_queue = sync_queue_new(10);
	pthread_create(&read_t, NULL, read_thread, (void *)data);
}

void destor_write(char *path, char *data) {
    /* 初始化recipe目录 */
	init_recipe_store();
	/* 创建一个container_buffer队列 */
	init_container_store();
	init_index();

	// 初始化backup job control record
	init_backup_jcr(path);

    puts("==== backup begin ====");

	TIMER_DECLARE(1);
	TIMER_BEGIN(1);

    time_t start = time(NULL);
	if (destor.simulation_level == SIMULATION_ALL) {
		start_read_trace_phase();
	} else {
		// 将jcr.path目录下的所有文件以块为单位读取出来压入到read_queue中
		start_read_phase_from_data();
		// 根据jcr.chunk_algorithm设置分块算法
		start_chunk_phase();
		// 从chunk_queue队列里pop出块，并计算该块的指纹，放到chunk.fp中
		start_hash_phase();
	}
	// 以segment为单位将chunk组织在一起，然后标记出duplicate的chunk
	// 最后将标记好的chunk推入dedup_queue
	start_dedup_phase();
	// 将dedup_queue弹出，并判断是否需要重写，将弹出的块以及需要重写的块push到rewrite_queue
	start_rewrite_phase();
	// 
	start_filter_phase();
}

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
            case 0:
                destor_write(cmd->_group_name, cmd->_data);
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